#include "game/EconomyService.hpp"

#include <algorithm>

namespace soma::game
{
    EconomyService::EconomyService(CampaignState& state)
        : state_(state)
    {
    }

    gameplay::Result<CampaignChange> EconomyService::grant(const gameplay::ItemDefinition& definition,
                                                            int quantity,
                                                            gameplay::ItemColor color)
    {
        color = gameplay::normalizedItemColor(color);
        if (quantity <= 0)
        {
            return gameplay::Result<CampaignChange>::failure("Item quantity must be positive");
        }
        if (const auto* currency = std::get_if<gameplay::CurrencyItem>(&definition.properties))
        {
            state_.currencies_[currency->currency] += quantity;
        }
        else if (definition.category == gameplay::ItemCategory::consumable)
        {
            const auto existing = state_.consumables_.find(definition.id);
            if (existing == state_.consumables_.end() && state_.consumables_.size() >= maximumDistinctConsumables)
            {
                return gameplay::Result<CampaignChange>::failure("The consumable inventory is full");
            }
            state_.consumables_[definition.id] += quantity;
        }
        else
        {
            for (auto index = 0; index < quantity; ++index)
            {
                state_.inventory_.push_back({definition.id, color});
            }
        }
        if (definition.category != gameplay::ItemCategory::currency)
        {
            state_.unlockedItems_.insert(definition.id);
        }
        return gameplay::Result<CampaignChange>::success(CampaignChange::itemAdded);
    }

    gameplay::Result<CampaignChange> EconomyService::purchase(const gameplay::ItemDefinition& definition,
                                                               int price,
                                                               gameplay::ItemColor color)
    {
        if (price < 0 || state_.currency(gameplay::Currency::cards) < price)
        {
            return gameplay::Result<CampaignChange>::failure("Not enough cards");
        }
        auto granted = grant(definition, 1, color);
        if (!granted.hasValue())
        {
            return granted;
        }
        state_.currencies_[gameplay::Currency::cards] -= price;
        return gameplay::Result<CampaignChange>::success(CampaignChange::itemPurchased);
    }

    gameplay::Result<CampaignChange> EconomyService::sell(const gameplay::ItemDefinition& definition,
                                                           const gameplay::ItemInstance& item,
                                                           int quantity)
    {
        if (quantity <= 0)
        {
            return gameplay::Result<CampaignChange>::failure("Item quantity must be positive");
        }
        if (definition.category == gameplay::ItemCategory::consumable)
        {
            const auto found = state_.consumables_.find(definition.id);
            if (found == state_.consumables_.end() || found->second < quantity)
            {
                return gameplay::Result<CampaignChange>::failure("The item is not in the inventory");
            }
            found->second -= quantity;
            if (found->second == 0)
            {
                state_.consumables_.erase(found);
            }
        }
        else
        {
            const auto found = std::ranges::find(state_.inventory_, item);
            if (found == state_.inventory_.end() || quantity != 1)
            {
                return gameplay::Result<CampaignChange>::failure("The item is not in the inventory");
            }
            state_.inventory_.erase(found);
        }
        state_.currencies_[gameplay::Currency::cards] += definition.price * quantity / 5;
        return gameplay::Result<CampaignChange>::success(CampaignChange::itemSold);
    }

    gameplay::Result<CampaignChange> EconomyService::equip(const EquipmentLocation& location, const gameplay::ItemInstance& item)
    {
        const auto found = std::ranges::find(state_.inventory_, item);
        if (found == state_.inventory_.end())
        {
            return gameplay::Result<CampaignChange>::failure("The item is not in the inventory");
        }
        const auto replacement = *found;
        state_.inventory_.erase(found);
        if (const auto equipped = state_.equipment_.find(location); equipped != state_.equipment_.end())
        {
            state_.inventory_.push_back(equipped->second);
            equipped->second = replacement;
        }
        else
        {
            state_.equipment_.emplace(location, replacement);
        }
        return gameplay::Result<CampaignChange>::success(CampaignChange::itemEquipped);
    }

    gameplay::Result<CampaignChange> EconomyService::unequip(const EquipmentLocation& location)
    {
        const auto found = state_.equipment_.find(location);
        if (found == state_.equipment_.end())
        {
            return gameplay::Result<CampaignChange>::failure("The equipment slot is empty");
        }
        state_.inventory_.push_back(found->second);
        state_.equipment_.erase(found);
        return gameplay::Result<CampaignChange>::success(CampaignChange::itemUnequipped);
    }

    gameplay::Result<CampaignChange> EconomyService::swapEquipment(const EquipmentLocation& first, const EquipmentLocation& second)
    {
        const auto firstItem = state_.equipment_.find(first);
        const auto secondItem = state_.equipment_.find(second);
        if (firstItem == state_.equipment_.end() || secondItem == state_.equipment_.end())
        {
            return gameplay::Result<CampaignChange>::failure("Both equipment slots must contain an item");
        }
        std::swap(firstItem->second, secondItem->second);
        return gameplay::Result<CampaignChange>::success(CampaignChange::itemEquipped);
    }

    gameplay::Result<CampaignChange> EconomyService::acquireSkill(const gameplay::SkillId& skill,
                                                                   int dicePrice,
                                                                   std::span<const gameplay::SkillId> prerequisites)
    {
        if (state_.acquiredSkills_.contains(skill))
        {
            return gameplay::Result<CampaignChange>::failure("The skill is already acquired");
        }
        if (dicePrice < 0 || state_.currency(gameplay::Currency::dice) < dicePrice)
        {
            return gameplay::Result<CampaignChange>::failure("Not enough dice");
        }
        if (!std::ranges::all_of(prerequisites, [this](const auto& prerequisite) { return state_.skillAcquired(prerequisite); }))
        {
            return gameplay::Result<CampaignChange>::failure("A prerequisite skill is missing");
        }
        state_.currencies_[gameplay::Currency::dice] -= dicePrice;
        state_.acquiredSkills_.insert(skill);
        return gameplay::Result<CampaignChange>::success(CampaignChange::skillAcquired);
    }

    gameplay::Result<CampaignChange> EconomyService::selectAbilityVersion(const gameplay::ContentDefinitionId& player,
                                                                           std::size_t abilitySlot,
                                                                           std::size_t version)
    {
        if (abilitySlot >= 4 || version >= 2)
        {
            return gameplay::Result<CampaignChange>::failure("Ability version selection is outside its supported range");
        }
        state_.activeAbilityVersions_.insert_or_assign({player, abilitySlot}, version);
        return gameplay::Result<CampaignChange>::success(CampaignChange::abilityVersionSelected);
    }

    gameplay::Result<CampaignChange> EconomyService::purchaseDice()
    {
        if (state_.currency(gameplay::Currency::cards) < state_.shopDicePrice_)
        {
            return gameplay::Result<CampaignChange>::failure("Not enough cards");
        }
        state_.currencies_[gameplay::Currency::cards] -= state_.shopDicePrice_;
        ++state_.currencies_[gameplay::Currency::dice];
        return gameplay::Result<CampaignChange>::success(CampaignChange::dicePurchased);
    }

    gameplay::Result<CampaignChange> EconomyService::unlockShopSlot()
    {
        if (state_.shopSlotsUnlocked_ >= maximumShopSlots)
        {
            return gameplay::Result<CampaignChange>::failure("All shop slots are already unlocked");
        }
        if (state_.currency(gameplay::Currency::cards) < shopUpgradeCost)
        {
            return gameplay::Result<CampaignChange>::failure("Not enough cards");
        }
        state_.currencies_[gameplay::Currency::cards] -= shopUpgradeCost;
        ++state_.shopSlotsUnlocked_;
        return gameplay::Result<CampaignChange>::success(CampaignChange::shopSlotUnlocked);
    }

    gameplay::Result<CampaignChange> EconomyService::reduceDicePrice()
    {
        if (state_.shopDicePrice_ <= minimumDicePrice)
        {
            return gameplay::Result<CampaignChange>::failure("The minimum dice price has been reached");
        }
        if (state_.currency(gameplay::Currency::cards) < shopUpgradeCost)
        {
            return gameplay::Result<CampaignChange>::failure("Not enough cards");
        }
        state_.currencies_[gameplay::Currency::cards] -= shopUpgradeCost;
        state_.shopDicePrice_ -= dicePriceReduction;
        return gameplay::Result<CampaignChange>::success(CampaignChange::dicePriceReduced);
    }

    gameplay::Result<CampaignChange> EconomyService::consume(const gameplay::ItemDefinition& definition, int quantity)
    {
        if (definition.category != gameplay::ItemCategory::consumable || quantity <= 0)
        {
            return gameplay::Result<CampaignChange>::failure("Only a positive quantity of consumables can be used");
        }
        const auto found = state_.consumables_.find(definition.id);
        if (found == state_.consumables_.end() || found->second < quantity)
        {
            return gameplay::Result<CampaignChange>::failure("The consumable is not in the inventory");
        }
        found->second -= quantity;
        if (found->second == 0)
        {
            state_.consumables_.erase(found);
        }
        return gameplay::Result<CampaignChange>::success(CampaignChange::itemConsumed);
    }

    void EconomyService::replaceShopStock(std::vector<ShopStockEntry> stock)
    {
        for (auto& entry : stock)
        {
            if (entry.item.has_value())
            {
                entry.item->color = gameplay::normalizedItemColor(entry.item->color);
            }
        }
        state_.shopStock_ = std::move(stock);
    }

    void EconomyService::removeShopStockEntry(const ShopStockEntry& entry)
    {
        const auto found = std::ranges::find(state_.shopStock_, entry);
        if (found != state_.shopStock_.end())
        {
            *found = {};
        }
    }

    void EconomyService::clearShopStock() noexcept
    {
        state_.shopStock_.clear();
    }
} // namespace soma::game
