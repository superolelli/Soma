#pragma once

#include "gameplay/items/ItemTypes.hpp"
#include "gameplay/rules/ComposedRule.hpp"

#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace soma::game
{
    struct EquipmentLocation
    {
        gameplay::ContentDefinitionId player;
        gameplay::EquipmentSlotId slot;

        auto operator<=>(const EquipmentLocation&) const = default;
    };

    struct EquipmentLocationHash
    {
        std::size_t operator()(const EquipmentLocation& location) const noexcept;
    };

    struct AbilitySetLocation
    {
        gameplay::ContentDefinitionId player;
        std::size_t slot{};

        auto operator<=>(const AbilitySetLocation&) const = default;
    };

    struct AbilitySetLocationHash
    {
        std::size_t operator()(const AbilitySetLocation& location) const noexcept;
    };

    struct ShopStockEntry
    {
        std::optional<gameplay::ItemInstance> item;
        std::optional<gameplay::RuleFragmentId> fragment;

        auto operator<=>(const ShopStockEntry&) const = default;
    };

    struct CampaignStateData
    {
        std::unordered_map<gameplay::Currency, int> currencies;
        std::vector<gameplay::ItemInstance> inventory;
        std::unordered_map<gameplay::ItemId, int> consumables;
        std::unordered_map<EquipmentLocation, gameplay::ItemInstance, EquipmentLocationHash> equipment;
        std::set<gameplay::ItemId> unlockedItems;
        std::set<gameplay::SkillId> acquiredSkills;
        std::unordered_map<AbilitySetLocation, std::size_t, AbilitySetLocationHash> activeAbilityVersions;
        std::unordered_map<gameplay::RuleFragmentId, int> ruleFragments;
        gameplay::ComposedRules rules;
        std::vector<ShopStockEntry> shopStock;
        int ruleSlotsUnlocked{1};
        int shopSlotsUnlocked{1};
        int shopDicePrice{100};
    };

    /// Owns all persistent player progression used by economy, rules, saves, and campaign flow.
    class CampaignState
    {
      public:
        [[nodiscard]] int currency(gameplay::Currency type) const noexcept;
        [[nodiscard]] int consumableCount(const gameplay::ItemId& item) const noexcept;
        [[nodiscard]] std::size_t distinctConsumables() const noexcept;
        [[nodiscard]] const std::vector<gameplay::ItemInstance>& inventory() const noexcept;
        [[nodiscard]] const gameplay::ItemInstance* equipped(const EquipmentLocation& location) const noexcept;
        [[nodiscard]] bool itemUnlocked(const gameplay::ItemId& item) const;
        [[nodiscard]] const std::set<gameplay::ItemId>& unlockedItems() const noexcept;
        [[nodiscard]] bool skillAcquired(const gameplay::SkillId& skill) const;
        [[nodiscard]] std::size_t activeAbilityVersion(const gameplay::ContentDefinitionId& player,
                                                       std::size_t abilitySlot) const noexcept;
        [[nodiscard]] int ruleFragmentCount(const gameplay::RuleFragmentId& fragment) const noexcept;
        [[nodiscard]] const gameplay::ComposedRules& rules() const noexcept;
        [[nodiscard]] const std::vector<ShopStockEntry>& shopStock() const noexcept;
        [[nodiscard]] int ruleSlotsUnlocked() const noexcept;
        [[nodiscard]] int shopSlotsUnlocked() const noexcept;
        [[nodiscard]] int shopDicePrice() const noexcept;
        [[nodiscard]] CampaignStateData data() const;
        [[nodiscard]] static CampaignState fromData(CampaignStateData data);

      private:
        friend class EconomyService;
        friend class RuleBuilderService;

        std::unordered_map<gameplay::Currency, int> currencies_{{gameplay::Currency::cards, 1000}, {gameplay::Currency::dice, 0}};
        std::vector<gameplay::ItemInstance> inventory_;
        std::unordered_map<gameplay::ItemId, int> consumables_;
        std::unordered_map<EquipmentLocation, gameplay::ItemInstance, EquipmentLocationHash> equipment_;
        std::set<gameplay::ItemId> unlockedItems_;
        std::set<gameplay::SkillId> acquiredSkills_;
        std::unordered_map<AbilitySetLocation, std::size_t, AbilitySetLocationHash> activeAbilityVersions_;
        std::unordered_map<gameplay::RuleFragmentId, int> ruleFragments_;
        gameplay::ComposedRules rules_;
        std::vector<ShopStockEntry> shopStock_;
        int ruleSlotsUnlocked_{1};
        int shopSlotsUnlocked_{1};
        int shopDicePrice_{100};
    };
} // namespace soma::game
