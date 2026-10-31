#include "game/EconomyQueries.hpp"
#include "presentation/sfml/economy/EconomyButton.hpp"
#include "presentation/sfml/economy/EconomyTooltip.hpp"
#include "presentation/sfml/economy/ShopPanel.hpp"

#include <algorithm>

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
            auto reducedPriceText =
                shopText(priceFont_, std::to_string(state_.shopDicePrice() - game::EconomyService::dicePurchasePriceReduction()), 14U, {});
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
        const auto& texture = textures_.at(content_.items.at(item.definition).icon);
        sf::RectangleShape color{sf::Vector2f{texture.getSize()}};
        color.setPosition(position);
        color.setFillColor({itemColor.red, itemColor.green, itemColor.blue});
        window_.draw(color);
        sf::Sprite sprite{texture};
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
