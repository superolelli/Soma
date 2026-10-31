#include "presentation/sfml/economy/ShopPanel.hpp"

#include "game/EconomyQueries.hpp"

#include <algorithm>
#include <ranges>

namespace soma::presentation
{
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
