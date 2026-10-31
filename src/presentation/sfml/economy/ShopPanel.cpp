#include "presentation/sfml/economy/ShopPanel.hpp"

#include "game/EconomyQueries.hpp"
#include "game/ShopStockGenerator.hpp"
#include "presentation/sfml/economy/EconomyButton.hpp"
#include "presentation/sfml/common/UiCursor.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation
{
    namespace
    {
        sf::Sprite shopButton(const sf::Texture& texture, sf::Vector2f position, unsigned int frames)
        {
            sf::Sprite sprite{texture};
            const auto width = texture.getSize().x / frames;
            sprite.setTextureRect(sf::IntRect{{0, 0}, {static_cast<int>(width), static_cast<int>(texture.getSize().y)}});
            sprite.setPosition(position);
            return sprite;
        }
    } // namespace

    ShopPanel::ShopPanel(sf::RenderWindow& window,
                         const content::EconomyContent& content,
                         const content::RuleContent& rules,
                         game::CampaignState& state,
                         game::EconomyService& economy,
                         game::RuleBuilderService& ruleService,
                         const EconomyTextures& textures,
                         const EconomyPanelFonts& fonts,
                         PanelHostOptions host)
        : window_(window)
        , content_(content)
        , rules_(rules)
        , state_(state)
        , economy_(economy)
        , ruleService_(ruleService)
        , textures_(textures)
        , interfaceFont_(fonts.interface)
        , titleFont_(fonts.title)
        , priceFont_(fonts.price)
        , closeWindowOnExit_(host.closeWindowOnExit)
        , drawBackground_(std::move(host.drawBackground))
        , navigate_(std::move(host.navigate))
        , navigationHovered_(std::move(host.navigationHovered))
    {
        for (const auto type : {content::RuleFragmentType::trigger,
                                content::RuleFragmentType::subject,
                                content::RuleFragmentType::effect,
                                content::RuleFragmentType::modifier})
        {
            ruleTextures_.emplace(type, sf::Texture{rules_.fragmentSprites.at(type)});
        }
        if (state_.shopStock().empty())
        {
            refillShopStock();
        }
    }

    void ShopPanel::refillShopStock()
    {
        economy_.replaceShopStock(game::ShopStockGenerator{random_}.generate(content_, rules_, state_, "bang", 5));
    }

    void ShopPanel::processEvents()
    {
        while (const auto event = window_.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window_.close();
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape)
            {
                requestClose();
            }
            if (const auto* moved = event->getIf<sf::Event::MouseMoved>())
            {
                mousePosition_ = window_.mapPixelToCoords(moved->position);
                if (sliderDragging_)
                {
                    const auto& texture = textures_.at("shop.sell_multiple");
                    updateSaleQuantity(mousePosition_, sellSliderBounds(dialogPosition(texture)));
                }
            }
            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                mousePosition_ = window_.mapPixelToCoords(wheel->position);
                const auto panelPosition = vendingPosition(content_.presentation.shop.inventoryPanel);
                const auto panelSize = textures_.at("inventory.items_panel").getSize();
                if (sf::FloatRect{panelPosition, {static_cast<float>(panelSize.x), static_cast<float>(panelSize.y)}}.contains(mousePosition_))
                {
                    const auto itemCount = state_.inventory().size();
                    const auto rows = (itemCount + 4U) / 5U;
                    const auto maximum = rows > 5U ? rows - 5U : 0U;
                    if (wheel->delta > 0.0F)
                    {
                        inventoryScrollRow_ = inventoryScrollRow_ == 0 ? 0 : inventoryScrollRow_ - 1;
                    }
                    else if (wheel->delta < 0.0F)
                    {
                        inventoryScrollRow_ = std::min(maximum, inventoryScrollRow_ + 1);
                    }
                }
            }
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>(); click && click->button == sf::Mouse::Button::Left)
            {
                mousePressed_ = true;
                mousePosition_ = window_.mapPixelToCoords(click->position);
                if (dialog_ == Dialog::sellMultiple)
                {
                    const auto& texture = textures_.at("shop.sell_multiple");
                    const auto bounds = sellSliderBounds(dialogPosition(texture));
                    const auto knobHeight = static_cast<float>(textures_.at("shop.slider_knob").getSize().y);
                    const sf::FloatRect interactionBounds{{bounds.position.x, bounds.position.y + (bounds.size.y - knobHeight) / 2.0F},
                                                          {bounds.size.x, knobHeight}};
                    sliderDragging_ = interactionBounds.contains(mousePosition_);
                    if (sliderDragging_)
                    {
                        updateSaleQuantity(mousePosition_, bounds);
                    }
                }
            }
            if (const auto* release = event->getIf<sf::Event::MouseButtonReleased>(); release && release->button == sf::Mouse::Button::Left)
            {
                mousePressed_ = false;
                mousePosition_ = window_.mapPixelToCoords(release->position);
                sliderDragging_ = false;
                if (dialog_ == Dialog::none && navigate_ && navigate_(mousePosition_))
                {
                    requestClose();
                }
                else
                {
                    handleClick(mousePosition_);
                }
            }
        }
    }

    bool ShopPanel::closeRequested() const noexcept
    {
        return closeRequested_;
    }

    void ShopPanel::requestClose()
    {
        closeRequested_ = true;
        if (closeWindowOnExit_)
        {
            window_.close();
        }
    }

    void ShopPanel::render()
    {
        window_.clear(sf::Color{30, 30, 30});
        if (drawBackground_)
        {
            drawBackground_();
        }
        drawPanels();
        drawSelection();
        drawShopOffers();
        drawOwnedItems();
        drawControls();
        drawTooltip();
        drawDialog();
        setUiCursor(window_, clickableAt(mousePosition_) || (navigationHovered_ && navigationHovered_(mousePosition_)));
        window_.display();
    }

    bool ShopPanel::clickableAt(sf::Vector2f position) const
    {
        if (dialog_ != Dialog::none)
        {
            return true;
        }
        const auto& layout = content_.presentation.shop;
        if (shopButton(textures_.at("shop.close"), vendingPosition(layout.closeButton), 3U).getGlobalBounds().contains(position) ||
            shopButton(textures_.at("skills.buy"), vendingPosition(layout.buyButton), 3U).getGlobalBounds().contains(position))
        {
            return true;
        }
        for (std::size_t index = 0; index < visibleShopOfferCount(); ++index)
        {
            const auto& offer = state_.shopStock().at(index);
            if ((offer.item || offer.fragment) && equipmentOfferBounds(index).contains(position))
            {
                return true;
            }
        }
        const auto offeredConsumables = consumableDefinitions();
        for (std::size_t index = 0; index < offeredConsumables.size(); ++index)
        {
            if (state_.itemUnlocked(offeredConsumables.at(index)) && consumableOfferBounds(index).contains(position))
            {
                return true;
            }
        }
        for (std::size_t index = 0; index < state_.inventory().size(); ++index)
        {
            if (inventoryBounds(index).contains(position))
            {
                return true;
            }
        }
        for (std::size_t index = 0; index < state_.distinctConsumables(); ++index)
        {
            if (consumableInventoryBounds(index).contains(position))
            {
                return true;
            }
        }
        return false;
    }

    void ShopPanel::handleClick(sf::Vector2f position)
    {
        if (dialog_ != Dialog::none)
        {
            handleDialogClick(position);
            return;
        }
        const auto& layout = content_.presentation.shop;
        if (shopButton(textures_.at("shop.close"), vendingPosition(layout.closeButton), 3U).getGlobalBounds().contains(position))
        {
            sounds_.click();
            requestClose();
            return;
        }
        const auto buy = shopButton(textures_.at("skills.buy"), vendingPosition(layout.buyButton), 3U);
        if (buy.getGlobalBounds().contains(position))
        {
            sounds_.click();
            executeSelection();
            return;
        }
        const auto nextSlot = state_.shopSlotsUnlocked();
        if (nextSlot < game::EconomyService::maximumShopSlotCount())
        {
            const auto lockPosition = shopPosition({layout.shopItemOrigin.x + nextSlot * layout.shopItemSpacing.x + layout.slotLockOffset.x,
                                                    layout.shopItemOrigin.y + layout.slotLockOffset.y});
            sf::Sprite lock{textures_.at("shop.slot_lock")};
            lock.setPosition(lockPosition);
            if (lock.getGlobalBounds().contains(position))
            {
                sounds_.click();
                dialog_ = Dialog::unlockSlot;
                return;
            }
        }
        sf::Sprite priceLock{textures_.at("shop.price_lock")};
        priceLock.setPosition(shopPosition(layout.dicePriceLock));
        if (priceLock.getGlobalBounds().contains(position))
        {
            sounds_.click();
            if (state_.shopDicePrice() > 50)
            {
                dialog_ = Dialog::reduceDicePrice;
            }
            return;
        }
        selectAt(position);
    }

    void ShopPanel::handleDialogClick(sf::Vector2f position)
    {
        const auto& panelTexture = textures_.at(dialog_ == Dialog::sellMultiple ? "shop.sell_multiple" : "shop.confirmation");
        const auto panelPosition = dialogPosition(panelTexture);
        if (dialog_ == Dialog::sellMultiple)
        {
            handleSellMultipleDialog(position, panelPosition);
            return;
        }
        handleConfirmationDialog(position, panelPosition);
    }

    void ShopPanel::handleSellMultipleDialog(sf::Vector2f position, sf::Vector2f panelPosition)
    {
        const auto& layout = content_.presentation.shop;
        const auto sliderBounds = sellSliderBounds(panelPosition);
        if (sliderBounds.contains(position) && selection_)
        {
            updateSaleQuantity(position, sliderBounds);
            return;
        }
        const auto buttonTextureSize = textures_.at("skills.buy").getSize();
        const sf::Vector2f buttonSize{static_cast<float>(buttonTextureSize.x / 3U), static_cast<float>(buttonTextureSize.y)};
        if (sf::FloatRect{panelPosition +
                              sf::Vector2f{static_cast<float>(layout.sellConfirmButton.x), static_cast<float>(layout.sellConfirmButton.y)},
                          buttonSize}
                .contains(position))
        {
            sounds_.click();
            sellSelectedConsumable();
        }
        else if (sf::FloatRect{panelPosition +
                                   sf::Vector2f{static_cast<float>(layout.sellCancelButton.x), static_cast<float>(layout.sellCancelButton.y)},
                               buttonSize}
                     .contains(position))
        {
            sounds_.click();
            dialog_ = Dialog::none;
        }
    }

    sf::FloatRect ShopPanel::sellSliderBounds(sf::Vector2f panelPosition) const
    {
        const auto sliderSize = textures_.at("shop.slider").getSize();
        const auto slider = content_.presentation.shop.sellSlider;
        return {panelPosition + sf::Vector2f{static_cast<float>(slider.x), static_cast<float>(slider.y)}, sf::Vector2f{sliderSize}};
    }

    void ShopPanel::updateSaleQuantity(sf::Vector2f position, const sf::FloatRect& sliderBounds)
    {
        if (!selection_)
        {
            return;
        }
        const auto maximum = state_.consumableCount(selection_->item.definition);
        const auto knobWidth = static_cast<float>(textures_.at("shop.slider_knob").getSize().x);
        const auto travel = sliderBounds.size.x - knobWidth;
        const auto progress = std::clamp((position.x - sliderBounds.position.x) / travel, 0.0F, 1.0F);
        saleQuantity_ = 1 + static_cast<int>(std::round(progress * static_cast<float>(maximum - 1)));
    }

    void ShopPanel::handleConfirmationDialog(sf::Vector2f position, sf::Vector2f panelPosition)
    {
        const auto& layout = content_.presentation.shop;
        const auto buttonTextureSize = textures_.at("skills.buy").getSize();
        const sf::Vector2f buttonSize{static_cast<float>(buttonTextureSize.x / 3U), static_cast<float>(buttonTextureSize.y)};
        const sf::FloatRect yes{
            panelPosition + sf::Vector2f{static_cast<float>(layout.confirmationYesButton.x), static_cast<float>(layout.confirmationYesButton.y)},
            buttonSize};
        const sf::FloatRect no{panelPosition +
                                   sf::Vector2f{static_cast<float>(layout.confirmationNoButton.x), static_cast<float>(layout.confirmationNoButton.y)},
                               buttonSize};
        if (yes.contains(position))
        {
            sounds_.click();
            if (dialog_ == Dialog::unlockSlot)
            {
                const auto result = economy_.unlockShopSlot();
                if (result.hasValue())
                {
                    sounds_.shopUpgrade();
                }
            }
            else if (dialog_ == Dialog::reduceDicePrice)
            {
                const auto result = economy_.reduceDicePrice();
                if (result.hasValue())
                {
                    sounds_.shopUpgrade();
                }
            }
            dialog_ = Dialog::none;
            selection_.reset();
        }
        else if (no.contains(position))
        {
            sounds_.click();
            dialog_ = Dialog::none;
        }
    }

} // namespace soma::presentation
