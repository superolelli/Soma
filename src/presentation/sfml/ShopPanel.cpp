#include "presentation/sfml/ShopPanel.hpp"

#include "game/EconomyQueries.hpp"
#include "game/ShopStockGenerator.hpp"
#include "presentation/sfml/EconomyButton.hpp"
#include "presentation/sfml/EconomyTooltip.hpp"
#include "presentation/sfml/UiCursor.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation
{
    namespace
    {
        sf::Text shopText(
            const sf::Font& font, const std::string& value, unsigned int size, sf::Vector2f position, sf::Color color = sf::Color::White)
        {
            sf::Text text{font, sf::String::fromUtf8(value.begin(), value.end()), size};
            text.setPosition(position);
            text.setFillColor(color);
            return text;
        }

        sf::Sprite shopButton(const sf::Texture& texture, sf::Vector2f position, unsigned int frames)
        {
            sf::Sprite sprite{texture};
            const auto width = texture.getSize().x / frames;
            sprite.setTextureRect(sf::IntRect{{0, 0}, {static_cast<int>(width), static_cast<int>(texture.getSize().y)}});
            sprite.setPosition(position);
            return sprite;
        }

        void centerText(sf::Text& text, const sf::FloatRect& bounds)
        {
            const auto textBounds = text.getLocalBounds();
            text.setPosition({bounds.position.x + (bounds.size.x - textBounds.size.x) / 2.0F - textBounds.position.x,
                              bounds.position.y + (bounds.size.y - textBounds.size.y) / 2.0F - textBounds.position.y});
        }

        sf::Text wrappedText(
            const sf::Font& font, const std::string& value, unsigned int size, sf::Vector2f position, float maximumWidth, sf::Color color)
        {
            std::string wrapped;
            std::string line;
            std::string word;
            const auto appendWord = [&]() {
                const auto candidate = line.empty() ? word : line + " " + word;
                auto measurement = shopText(font, candidate, size, {});
                if (!line.empty() && measurement.getLocalBounds().size.x > maximumWidth)
                {
                    wrapped += line + '\n';
                    line = word;
                }
                else
                {
                    line = candidate;
                }
                word.clear();
            };
            for (const auto character : value)
            {
                if (character == ' ')
                {
                    appendWord();
                }
                else
                {
                    word += character;
                }
            }
            appendWord();
            wrapped += line;
            return shopText(font, wrapped, size, position, color);
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

    void ShopPanel::drawPanels() const
    {
        sf::Sprite vending{textures_.at("shop.vending_panel")};
        vending.setPosition(vendingPosition({0, 0}));
        window_.draw(vending);
        sf::Sprite inventory{textures_.at("inventory.items_panel")};
        inventory.setPosition(vendingPosition(content_.presentation.shop.inventoryPanel));
        window_.draw(inventory);
        sf::Sprite row{textures_.at("inventory.item_row")};
        row.setPosition(vendingPosition(content_.presentation.shop.consumablesRow));
        window_.draw(row);
        sf::Sprite shop{textures_.at("shop.panel")};
        shop.setPosition(shopPosition({0, 0}));
        window_.draw(shop);
        auto offerTitle = shopText(titleFont_, "Nur für kurze Zeit!", 50U, shopPosition(content_.presentation.shop.offerTitle), sf::Color::Black);
        window_.draw(offerTitle);
        window_.draw(shopText(priceFont_, "Shop", 50U, vendingPosition(content_.presentation.shop.title), sf::Color::Black));
    }

    void ShopPanel::drawShopOffers() const
    {
        const auto& stock = state_.shopStock();
        for (std::size_t index = 0; index < visibleShopOfferCount(); ++index)
        {
            const auto position = equipmentOfferBounds(index).position;
            const auto& offer = stock.at(index);
            if (offer.item)
            {
                drawItem(*offer.item, position);
                drawPrice(content_.items.at(offer.item->definition).price, position);
            }
            else if (offer.fragment)
            {
                const auto& definition = rules_.fragments.at(*offer.fragment);
                const auto bounds = equipmentOfferBounds(index);
                sf::Sprite icon{ruleTextures_.at(definition.type)};
                const auto textureSize = icon.getTexture().getSize();
                const auto scale = bounds.size.x / static_cast<float>(std::max(textureSize.x, textureSize.y));
                icon.setScale({scale, scale});
                icon.setPosition(bounds.position);
                window_.draw(icon);
                drawPrice(rules_.shopPrice, position);
            }
        }
        const auto consumables = consumableDefinitions();
        for (std::size_t index = 0; index < consumables.size(); ++index)
        {
            const auto position = consumableOfferBounds(index).position;
            const auto unlocked = state_.itemUnlocked(consumables.at(index));
            drawItem({consumables.at(index), {}}, position, !unlocked);
            drawPrice(content_.items.at(consumables.at(index)).price, position);
        }
        const auto dicePosition = shopPosition(content_.presentation.shop.dice);
        drawItem({gameplay::ItemId{"currency.dice"}, {}}, dicePosition);
        drawPrice(state_.shopDicePrice(), dicePosition);
        for (auto slot = 4; slot >= state_.shopSlotsUnlocked(); --slot)
        {
            sf::Sprite lock{textures_.at("shop.slot_lock")};
            const auto& layout = content_.presentation.shop;
            lock.setPosition(shopPosition({layout.shopItemOrigin.x + slot * layout.shopItemSpacing.x + layout.slotLockOffset.x,
                                           layout.shopItemOrigin.y + layout.slotLockOffset.y}));
            window_.draw(lock);
            if (slot == state_.shopSlotsUnlocked())
            {
                auto price = shopText(priceFont_, std::to_string(game::EconomyService::shopUpgradePrice()), 14U, {}, sf::Color::White);
                const auto& bounds = layout.slotUnlockPrice;
                centerText(price,
                           {lock.getPosition() + sf::Vector2f{static_cast<float>(bounds.x), static_cast<float>(bounds.y)},
                            {static_cast<float>(bounds.width), static_cast<float>(bounds.height)}});
                window_.draw(price);
            }
        }
        if (state_.shopDicePrice() > 50)
        {
            sf::Sprite lock{textures_.at("shop.price_lock")};
            lock.setPosition(shopPosition(content_.presentation.shop.dicePriceLock));
            sf::Sprite reducedPrice{textures_.at("shop.price")};
            reducedPrice.setPosition(lock.getPosition() + sf::Vector2f{static_cast<float>(content_.presentation.shop.reducedPriceSignOffset.x),
                                                                       static_cast<float>(content_.presentation.shop.reducedPriceSignOffset.y)});
            window_.draw(reducedPrice);
            auto reducedPriceText = shopText(
                priceFont_, std::to_string(state_.shopDicePrice() - game::EconomyService::dicePurchasePriceReduction()), 14U, {});
            centerText(reducedPriceText, reducedPrice.getGlobalBounds());
            window_.draw(reducedPriceText);
            window_.draw(lock);
            auto unlockPrice = shopText(priceFont_, std::to_string(game::EconomyService::shopUpgradePrice()), 14U, {}, sf::Color::White);
            const auto& bounds = content_.presentation.shop.diceUnlockPrice;
            centerText(unlockPrice,
                       {lock.getPosition() + sf::Vector2f{static_cast<float>(bounds.x), static_cast<float>(bounds.y)},
                        {static_cast<float>(bounds.width), static_cast<float>(bounds.height)}});
            window_.draw(unlockPrice);
        }
    }

    void ShopPanel::drawSelection() const
    {
        if (const auto selectedPosition = selectionPosition())
        {
            sf::Sprite frame{textures_.at("shop.selection")};
            const auto inset = content_.presentation.shop.selectionInset;
            frame.setPosition(*selectedPosition + sf::Vector2f{static_cast<float>(inset.x), static_cast<float>(inset.y)});
            window_.draw(frame);
        }
    }

    void ShopPanel::drawOwnedItems() const
    {
        const auto inventory = game::EconomyQueries{state_, content_}.inventory(game::ItemSort::name);
        const auto first = inventoryScrollRow_ * 5U;
        const auto last = std::min(inventory.size(), first + 25U);
        for (auto index = first; index < last; ++index)
        {
            drawItem(inventory.at(index).item, inventoryBounds(index).position);
        }
        const auto consumables = game::EconomyQueries{state_, content_}.consumables(game::ItemSort::name);
        for (std::size_t index = 0; index < consumables.size(); ++index)
        {
            const auto position = consumableInventoryBounds(index).position;
            drawItem({consumables.at(index).item, {}}, position);
            const auto offset = content_.presentation.shop.quantityOffset;
            window_.draw(shopText(interfaceFont_,
                                  std::to_string(consumables.at(index).quantity),
                                  16U,
                                  position + sf::Vector2f{static_cast<float>(offset.x), static_cast<float>(offset.y)}));
        }
    }

    void ShopPanel::drawControls() const
    {
        const auto& layout = content_.presentation.shop;
        window_.draw(economyButton(
            textures_.at("shop.close"), vendingPosition(layout.closeButton), EconomyButtonBehavior::motionUp, mousePosition_, mousePressed_));
        auto action = economyButton(textures_.at("skills.buy"),
                                    vendingPosition(layout.buyButton),
                                    EconomyButtonBehavior::motionUp,
                                    mousePosition_,
                                    mousePressed_,
                                    selection_.has_value());
        window_.draw(action);
        const auto selling =
            selection_ && (selection_->source == SelectionSource::inventory || selection_->source == SelectionSource::consumableInventory);
        const auto actionBounds = action.getGlobalBounds();
        auto actionLabel = shopText(interfaceFont_, selling ? "Verkaufen" : "Kaufen", 25U, {}, sf::Color::Black);
        centerText(actionLabel, actionBounds);
        window_.draw(actionLabel);
        if (selection_)
        {
            const auto value = selection_->source == SelectionSource::ruleOffer ? rules_.shopPrice
                               : selection_->source == SelectionSource::dice    ? state_.shopDicePrice()
                                                                                : (selling ? content_.items.at(selection_->item.definition).price / 5
                                                                                           : content_.items.at(selection_->item.definition).price);
            window_.draw(shopText(titleFont_,
                                  "Für " + std::to_string(value),
                                  25U,
                                  vendingPosition({layout.buyButton.x + layout.actionPrice.x, layout.buyButton.y + layout.actionPrice.y}),
                                  sf::Color::Black));
            sf::Sprite cards{textures_.at("shop.cards")};
            cards.setPosition(vendingPosition({layout.buyButton.x + layout.actionCards.x, layout.buyButton.y + layout.actionCards.y}));
            window_.draw(cards);
        }
    }

    void ShopPanel::drawTooltip() const
    {
        if (dialog_ != Dialog::none)
        {
            return;
        }
        const auto& stock = state_.shopStock();
        for (std::size_t index = 0; index < visibleShopOfferCount(); ++index)
        {
            if (equipmentOfferBounds(index).contains(mousePosition_))
            {
                const auto& offer = stock.at(index);
                if (offer.item)
                {
                    drawItemTooltip(window_, interfaceFont_, content_.items.at(offer.item->definition), mousePosition_);
                }
                else if (offer.fragment)
                {
                    drawRuleFragmentTooltip(window_, interfaceFont_, rules_.fragments.at(*offer.fragment), mousePosition_);
                }
                return;
            }
        }
        const auto offeredConsumables = consumableDefinitions();
        for (std::size_t index = 0; index < offeredConsumables.size(); ++index)
        {
            if (consumableOfferBounds(index).contains(mousePosition_) && state_.itemUnlocked(offeredConsumables.at(index)))
            {
                drawItemTooltip(window_, interfaceFont_, content_.items.at(offeredConsumables.at(index)), mousePosition_);
                return;
            }
        }
        const auto inventory = game::EconomyQueries{state_, content_}.inventory(game::ItemSort::name);
        const auto first = inventoryScrollRow_ * 5U;
        const auto last = std::min(inventory.size(), first + 25U);
        for (auto index = first; index < last; ++index)
        {
            if (inventoryBounds(index).contains(mousePosition_))
            {
                drawItemTooltip(window_, interfaceFont_, content_.items.at(inventory.at(index).item.definition), mousePosition_);
                return;
            }
        }
        const auto consumables = game::EconomyQueries{state_, content_}.consumables(game::ItemSort::name);
        for (std::size_t index = 0; index < consumables.size(); ++index)
        {
            if (consumableInventoryBounds(index).contains(mousePosition_))
            {
                drawItemTooltip(window_, interfaceFont_, content_.items.at(consumables.at(index).item), mousePosition_);
                return;
            }
        }
    }

    void ShopPanel::drawDialog() const
    {
        if (dialog_ == Dialog::none)
        {
            return;
        }
        const auto sellMultiple = dialog_ == Dialog::sellMultiple;
        const auto& texture = textures_.at(sellMultiple ? "shop.sell_multiple" : "shop.confirmation");
        const auto position = dialogPosition(texture);
        sf::Sprite panel{texture};
        panel.setPosition(position);
        window_.draw(panel);
        if (sellMultiple)
        {
            drawSellMultipleDialog(position);
            return;
        }
        drawConfirmationDialog(position);
    }

    void ShopPanel::drawSellMultipleDialog(sf::Vector2f position) const
    {
        const auto& layout = content_.presentation.shop;
        sf::Sprite slider{textures_.at("shop.slider")};
        slider.setPosition(position + sf::Vector2f{static_cast<float>(layout.sellSlider.x), static_cast<float>(layout.sellSlider.y)});
        window_.draw(slider);
        const auto maximum = selection_ ? state_.consumableCount(selection_->item.definition) : 1;
        const auto progress = maximum <= 1 ? 0.0F : static_cast<float>(saleQuantity_ - 1) / static_cast<float>(maximum - 1);
        sf::Sprite knob{textures_.at("shop.slider_knob")};
        const auto sliderBounds = slider.getGlobalBounds();
        const auto knobSize = sf::Vector2f{knob.getTexture().getSize()};
        knob.setPosition({sliderBounds.position.x + progress * (sliderBounds.size.x - knobSize.x),
                          sliderBounds.position.y + (sliderBounds.size.y - knobSize.y) / 2.0F});
        window_.draw(knob);
        drawDialogButton("Verkaufen",
                         position + sf::Vector2f{static_cast<float>(layout.sellConfirmButton.x), static_cast<float>(layout.sellConfirmButton.y)},
                         20U);
        drawDialogButton(
            "Abbrechen", position + sf::Vector2f{static_cast<float>(layout.sellCancelButton.x), static_cast<float>(layout.sellCancelButton.y)}, 20U);
        const auto& quantityBounds = layout.sellQuantityBounds;
        auto quantity = shopText(priceFont_, std::to_string(saleQuantity_), 18U, {}, sf::Color::Black);
        centerText(quantity,
                   {position + sf::Vector2f{static_cast<float>(quantityBounds.x), static_cast<float>(quantityBounds.y)},
                    {static_cast<float>(quantityBounds.width), static_cast<float>(quantityBounds.height)}});
        window_.draw(quantity);
    }

    void ShopPanel::drawConfirmationDialog(sf::Vector2f position) const
    {
        const auto& layout = content_.presentation.shop;
        const auto price = std::to_string(game::EconomyService::shopUpgradePrice());
        const auto reduction = std::to_string(game::EconomyService::dicePurchasePriceReduction());
        const auto message = dialog_ == Dialog::unlockSlot ? "Shop-Slot für " + price + " Karten freischalten?"
                                                           : "Würfelpreis für " + price + " Karten um " + reduction + " reduzieren?";
        window_.draw(
            wrappedText(interfaceFont_,
                        message,
                        22U,
                        position + sf::Vector2f{static_cast<float>(layout.confirmationMessage.x), static_cast<float>(layout.confirmationMessage.y)},
                        static_cast<float>(layout.confirmationMessageWidth),
                        sf::Color::Black));
        drawDialogButton("Ja",
                         position +
                             sf::Vector2f{static_cast<float>(layout.confirmationYesButton.x), static_cast<float>(layout.confirmationYesButton.y)},
                         23U);
        drawDialogButton("Nein",
                         position +
                             sf::Vector2f{static_cast<float>(layout.confirmationNoButton.x), static_cast<float>(layout.confirmationNoButton.y)},
                         23U);
    }

    void ShopPanel::drawDialogButton(std::string_view label, sf::Vector2f position, unsigned int characterSize) const
    {
        auto button = economyButton(textures_.at("skills.buy"), position, EconomyButtonBehavior::motionUp, mousePosition_, mousePressed_);
        window_.draw(button);
        auto text = shopText(interfaceFont_, std::string{label}, characterSize, {}, sf::Color::Black);
        centerText(text, button.getGlobalBounds());
        window_.draw(text);
    }

    void ShopPanel::drawItem(const gameplay::ItemInstance& item, sf::Vector2f position, bool dimmed) const
    {
        const auto itemColor = gameplay::normalizedItemColor(item.color);
        sf::RectangleShape color{{97.0F, 97.0F}};
        color.setPosition(position + sf::Vector2f{2.0F, 2.0F});
        color.setFillColor({itemColor.red, itemColor.green, itemColor.blue});
        window_.draw(color);
        sf::Sprite sprite{textures_.at(content_.items.at(item.definition).icon)};
        sprite.setPosition(position);
        if (dimmed)
        {
            sprite.setColor(sf::Color{40, 40, 40});
        }
        window_.draw(sprite);
    }

    void ShopPanel::drawPrice(int price, sf::Vector2f itemPosition) const
    {
        sf::Sprite sign{textures_.at("shop.price")};
        const auto offset = content_.presentation.shop.priceSignOffset;
        sign.setPosition(itemPosition + sf::Vector2f{static_cast<float>(offset.x), static_cast<float>(offset.y)});
        window_.draw(sign);
        auto text = shopText(priceFont_, std::to_string(price), 14U, {});
        centerText(text, sign.getGlobalBounds());
        window_.draw(text);
    }

    sf::Vector2f ShopPanel::dialogPosition(const sf::Texture& texture) const
    {
        const auto referenceSize = content_.presentation.referenceSize;
        return {static_cast<float>(referenceSize.x - static_cast<int>(texture.getSize().x)) / 2.0F,
                static_cast<float>(referenceSize.y - static_cast<int>(texture.getSize().y)) / 2.0F};
    }
} // namespace soma::presentation
