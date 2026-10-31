#include "game/CampaignState.hpp"

namespace soma::game
{
    std::size_t EquipmentLocationHash::operator()(const EquipmentLocation& location) const noexcept
    {
        const auto playerHash = std::hash<gameplay::ContentDefinitionId>{}(location.player);
        const auto slotHash = std::hash<gameplay::EquipmentSlotId>{}(location.slot);
        return playerHash ^ (slotHash << 1U);
    }

    std::size_t AbilitySetLocationHash::operator()(const AbilitySetLocation& location) const noexcept
    {
        return std::hash<gameplay::ContentDefinitionId>{}(location.player) ^ (std::hash<std::size_t>{}(location.slot) << 1U);
    }

    int CampaignState::currency(gameplay::Currency type) const noexcept
    {
        const auto found = currencies_.find(type);
        return found == currencies_.end() ? 0 : found->second;
    }

    int CampaignState::consumableCount(const gameplay::ItemId& item) const noexcept
    {
        const auto found = consumables_.find(item);
        return found == consumables_.end() ? 0 : found->second;
    }

    std::size_t CampaignState::distinctConsumables() const noexcept
    {
        return consumables_.size();
    }

    const std::vector<gameplay::ItemInstance>& CampaignState::inventory() const noexcept
    {
        return inventory_;
    }

    const gameplay::ItemInstance* CampaignState::equipped(const EquipmentLocation& location) const noexcept
    {
        const auto found = equipment_.find(location);
        return found == equipment_.end() ? nullptr : &found->second;
    }

    bool CampaignState::itemUnlocked(const gameplay::ItemId& item) const
    {
        return unlockedItems_.contains(item);
    }

    const std::set<gameplay::ItemId>& CampaignState::unlockedItems() const noexcept
    {
        return unlockedItems_;
    }

    bool CampaignState::skillAcquired(const gameplay::SkillId& skill) const
    {
        return acquiredSkills_.contains(skill);
    }

    std::size_t CampaignState::activeAbilityVersion(const gameplay::ContentDefinitionId& player,
                                                     std::size_t abilitySlot) const noexcept
    {
        const auto found = activeAbilityVersions_.find({player, abilitySlot});
        return found == activeAbilityVersions_.end() ? 0U : found->second;
    }

    int CampaignState::ruleFragmentCount(const gameplay::RuleFragmentId& fragment) const noexcept
    {
        const auto found = ruleFragments_.find(fragment);
        return found == ruleFragments_.end() ? 0 : found->second;
    }

    const gameplay::ComposedRules& CampaignState::rules() const noexcept
    {
        return rules_;
    }

    const std::vector<ShopStockEntry>& CampaignState::shopStock() const noexcept
    {
        return shopStock_;
    }

    int CampaignState::ruleSlotsUnlocked() const noexcept
    {
        return ruleSlotsUnlocked_;
    }

    int CampaignState::shopSlotsUnlocked() const noexcept
    {
        return shopSlotsUnlocked_;
    }

    int CampaignState::shopDicePrice() const noexcept
    {
        return shopDicePrice_;
    }

    CampaignStateData CampaignState::data() const
    {
        return {currencies_,
                inventory_,
                consumables_,
                equipment_,
                unlockedItems_,
                acquiredSkills_,
                activeAbilityVersions_,
                ruleFragments_,
                rules_,
                shopStock_,
                ruleSlotsUnlocked_,
                shopSlotsUnlocked_,
                shopDicePrice_};
    }

    CampaignState CampaignState::fromData(CampaignStateData data)
    {
        for (auto& item : data.inventory)
        {
            item.color = gameplay::normalizedItemColor(item.color);
        }
        for (auto& [location, item] : data.equipment)
        {
            static_cast<void>(location);
            item.color = gameplay::normalizedItemColor(item.color);
        }
        for (auto& entry : data.shopStock)
        {
            if (entry.item.has_value())
            {
                entry.item->color = gameplay::normalizedItemColor(entry.item->color);
            }
        }
        CampaignState result;
        result.currencies_ = std::move(data.currencies);
        result.inventory_ = std::move(data.inventory);
        result.consumables_ = std::move(data.consumables);
        result.equipment_ = std::move(data.equipment);
        result.unlockedItems_ = std::move(data.unlockedItems);
        result.acquiredSkills_ = std::move(data.acquiredSkills);
        result.activeAbilityVersions_ = std::move(data.activeAbilityVersions);
        result.ruleFragments_ = std::move(data.ruleFragments);
        result.rules_ = std::move(data.rules);
        result.shopStock_ = std::move(data.shopStock);
        result.ruleSlotsUnlocked_ = data.ruleSlotsUnlocked;
        result.shopSlotsUnlocked_ = data.shopSlotsUnlocked;
        result.shopDicePrice_ = data.shopDicePrice;
        return result;
    }
} // namespace soma::game
