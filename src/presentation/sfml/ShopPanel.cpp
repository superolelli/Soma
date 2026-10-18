#include "presentation/sfml/ShopPanel.hpp"

#include "game/EconomyQueries.hpp"
#include "game/ShopStockGenerator.hpp"
#include "presentation/sfml/EconomyButton.hpp"
#include "presentation/sfml/UiCursor.hpp"

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
                         const sf::Font& interfaceFont,
                         const sf::Font& titleFont,
                         const sf::Font& priceFont,
                         bool closeWindowOnExit,
                         std::function<void()> drawBackground,
                         std::function<bool(sf::Vector2f)> navigate,
                         std::function<bool(sf::Vector2f)> navigationHovered)
        : window_(window)
        , content_(content)
        , rules_(rules)
        , state_(state)
        , economy_(economy)
        , ruleService_(ruleService)
        , textures_(textures)
        , interfaceFont_(interfaceFont)
        , titleFont_(titleFont)
        , priceFont_(priceFont)
        , closeWindowOnExit_(closeWindowOnExit)
        , drawBackground_(std::move(drawBackground))
        , navigate_(std::move(navigate))
        , navigationHovered_(std::move(navigationHovered))
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

    std::size_t ShopPanel::visibleShopOfferCount() const noexcept
    {
        return std::min(state_.shopStock().size(), static_cast<std::size_t>(state_.shopSlotsUnlocked()));
    }

    sf::Vector2f ShopPanel::vendingPosition(content::Point relative) const
    {
        const auto origin = content_.presentation.shop.vendingOrigin;
        return {static_cast<float>(origin.x + relative.x), static_cast<float>(origin.y + relative.y)};
    }

    sf::Vector2f ShopPanel::shopPosition(content::Point relative) const
    {
        const auto shop = content_.presentation.shop.shopPanel;
        return vendingPosition({shop.x + relative.x, shop.y + relative.y});
    }

    std::vector<gameplay::ItemId> ShopPanel::consumableDefinitions() const
    {
        std::vector<gameplay::ItemId> result;
        for (const auto& [id, item] : content_.items)
        {
            if (item.category == gameplay::ItemCategory::consumable)
            {
                result.push_back(id);
            }
        }
        std::ranges::sort(result, {}, [this](const auto& id) {
            return content_.items.at(id).name;
        });
        return result;
    }

    sf::FloatRect ShopPanel::equipmentOfferBounds(std::size_t index) const
    {
        const auto& layout = content_.presentation.shop;
        return {shopPosition({layout.shopItemOrigin.x + static_cast<int>(index) * layout.shopItemSpacing.x, layout.shopItemOrigin.y}),
                {100.0F, 100.0F}};
    }

    sf::FloatRect ShopPanel::consumableOfferBounds(std::size_t index) const
    {
        const auto& layout = content_.presentation.shop;
        const auto column = static_cast<int>(index % 5);
        const auto row = static_cast<int>(index / 5);
        return {shopPosition(
                    {layout.consumableOrigin.x + column * layout.consumableSpacing.x, layout.consumableOrigin.y + row * layout.consumableSpacing.y}),
                {100.0F, 100.0F}};
    }

    sf::FloatRect ShopPanel::inventoryBounds(std::size_t index) const
    {
        const auto origin = vendingPosition(content_.presentation.shop.inventoryPanel);
        const auto& layout = content_.presentation.shop;
        const auto visibleIndex = index - inventoryScrollRow_ * 5U;
        return {origin + sf::Vector2f{static_cast<float>(layout.ownedItemInset.x + (visibleIndex % 5) * layout.ownedItemSpacing.x),
                                      static_cast<float>(layout.ownedItemInset.y + (visibleIndex / 5) * layout.ownedItemSpacing.y)},
                {static_cast<float>(layout.itemSize.x), static_cast<float>(layout.itemSize.y)}};
    }

    sf::FloatRect ShopPanel::consumableInventoryBounds(std::size_t index) const
    {
        const auto origin = vendingPosition(content_.presentation.shop.consumablesRow);
        const auto& layout = content_.presentation.shop;
        return {origin + sf::Vector2f{static_cast<float>(layout.ownedItemInset.x + index * layout.ownedItemSpacing.x),
                                      static_cast<float>(layout.ownedItemInset.y)},
                {static_cast<float>(layout.itemSize.x), static_cast<float>(layout.itemSize.y)}};
    }

    std::optional<sf::Vector2f> ShopPanel::selectionPosition() const
    {
        if (!selection_)
        {
            return std::nullopt;
        }
        if (selection_->source == SelectionSource::equipmentOffer)
        {
            const auto& stock = state_.shopStock();
            const auto found = std::ranges::find_if(stock, [this](const game::ShopStockEntry& offer) {
                return offer.item && *offer.item == selection_->item;
            });
            if (found != stock.end())
            {
                return equipmentOfferBounds(static_cast<std::size_t>(std::distance(stock.begin(), found))).position;
            }
        }
        if (selection_->source == SelectionSource::consumableOffer)
        {
            const auto consumables = consumableDefinitions();
            const auto found = std::ranges::find(consumables, selection_->item.definition);
            if (found != consumables.end())
            {
                return consumableOfferBounds(static_cast<std::size_t>(std::distance(consumables.begin(), found))).position;
            }
        }
        if (selection_->source == SelectionSource::dice)
        {
            return shopPosition(content_.presentation.shop.dice);
        }
        if (selection_->source == SelectionSource::inventory)
        {
            const auto inventory = game::EconomyQueries{state_, content_}.inventory(game::ItemSort::name);
            const auto found = std::ranges::find(inventory, selection_->item, &game::InventoryEntry::item);
            if (found != inventory.end())
            {
                const auto index = static_cast<std::size_t>(std::distance(inventory.begin(), found));
                const auto first = inventoryScrollRow_ * 5U;
                if (index >= first && index < first + 25U)
                {
                    return inventoryBounds(index).position;
                }
            }
        }
        if (selection_->source == SelectionSource::consumableInventory)
        {
            const auto consumables = game::EconomyQueries{state_, content_}.consumables(game::ItemSort::name);
            const auto found = std::ranges::find(consumables, selection_->item.definition, &game::ConsumableEntry::item);
            if (found != consumables.end())
            {
                return consumableInventoryBounds(static_cast<std::size_t>(std::distance(consumables.begin(), found))).position;
            }
        }
        if (selection_->source == SelectionSource::ruleOffer && selection_->fragment)
        {
            const auto& stock = state_.shopStock();
            const auto found = std::ranges::find_if(stock, [this](const game::ShopStockEntry& offer) {
                return offer.fragment && *offer.fragment == *selection_->fragment;
            });
            if (found != stock.end())
            {
                return equipmentOfferBounds(static_cast<std::size_t>(std::distance(stock.begin(), found))).position;
            }
        }
        return std::nullopt;
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

    void ShopPanel::selectAt(sf::Vector2f position)
    {
        const auto& stock = state_.shopStock();
        for (std::size_t index = 0; index < visibleShopOfferCount(); ++index)
        {
            if (equipmentOfferBounds(index).contains(position))
            {
                const auto& offer = stock.at(index);
                if (offer.item)
                {
                    selection_ = Selection{SelectionSource::equipmentOffer, *offer.item};
                }
                else if (offer.fragment)
                {
                    selection_ = Selection{SelectionSource::ruleOffer, {}, offer.fragment};
                }
                else
                {
                    selection_.reset();
                }
                return;
            }
        }
        const auto consumables = consumableDefinitions();
        for (std::size_t index = 0; index < consumables.size(); ++index)
        {
            if (consumableOfferBounds(index).contains(position))
            {
                selection_ = Selection{SelectionSource::consumableOffer, {consumables.at(index), {}}};
                return;
            }
        }
        const auto diceBounds = sf::FloatRect{shopPosition(content_.presentation.shop.dice), {100.0F, 100.0F}};
        if (diceBounds.contains(position))
        {
            selection_ = Selection{SelectionSource::dice, {gameplay::ItemId{"currency.dice"}, {}}};
            return;
        }
        const auto inventory = game::EconomyQueries{state_, content_}.inventory(game::ItemSort::name);
        const auto first = inventoryScrollRow_ * 5U;
        const auto last = std::min(inventory.size(), first + 25U);
        for (auto index = first; index < last; ++index)
        {
            if (inventoryBounds(index).contains(position))
            {
                selection_ = Selection{SelectionSource::inventory, inventory.at(index).item};
                return;
            }
        }
        const auto ownedConsumables = game::EconomyQueries{state_, content_}.consumables(game::ItemSort::name);
        for (std::size_t index = 0; index < ownedConsumables.size(); ++index)
        {
            if (consumableInventoryBounds(index).contains(position))
            {
                selection_ = Selection{SelectionSource::consumableInventory, {ownedConsumables.at(index).item, {}}};
                return;
            }
        }
        selection_.reset();
    }

    void ShopPanel::executeSelection()
    {
        if (!selection_)
        {
            return;
        }
        const auto source = selection_->source;
        if (source == SelectionSource::ruleOffer && selection_->fragment)
        {
            const auto purchased = ruleService_.purchase(*selection_->fragment);
            if (purchased.hasValue())
            {
                economy_.removeShopStockEntry({std::nullopt, selection_->fragment});
            }
        }
        else if (source == SelectionSource::dice)
        {
            static_cast<void>(economy_.purchaseDice());
        }
        else if (source == SelectionSource::consumableInventory && state_.consumableCount(selection_->item.definition) > 1)
        {
            saleQuantity_ = 1;
            dialog_ = Dialog::sellMultiple;
            return;
        }
        else if (source == SelectionSource::inventory || source == SelectionSource::consumableInventory)
        {
            static_cast<void>(economy_.sell(content_.items.at(selection_->item.definition), selection_->item));
        }
        else
        {
            const auto& definition = content_.items.at(selection_->item.definition);
            if (source != SelectionSource::consumableOffer || state_.itemUnlocked(definition.id))
            {
                const auto purchased = economy_.purchase(definition, definition.price, selection_->item.color);
                if (purchased.hasValue() && source == SelectionSource::equipmentOffer)
                {
                    economy_.removeShopStockEntry({selection_->item, std::nullopt});
                }
            }
        }
        selection_.reset();
    }

    void ShopPanel::sellSelectedConsumable()
    {
        if (selection_ && selection_->source == SelectionSource::consumableInventory)
        {
            const auto& definition = content_.items.at(selection_->item.definition);
            static_cast<void>(economy_.sell(definition, selection_->item, saleQuantity_));
        }
        selection_.reset();
        dialog_ = Dialog::none;
    }

} // namespace soma::presentation
