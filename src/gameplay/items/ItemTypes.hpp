#pragma once

#include "gameplay/common/StrongId.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace soma::gameplay
{
    enum class ItemCategory
    {
        currency,
        equipment,
        consumable
    };

    enum class Currency
    {
        cards,
        dice
    };

    struct ItemColor
    {
        std::uint8_t red{};
        std::uint8_t green{};
        std::uint8_t blue{};

        auto operator<=>(const ItemColor&) const = default;
    };

    [[nodiscard]] constexpr bool isSupportedItemColor(ItemColor color) noexcept
    {
        return color == ItemColor{128, 0, 0} || color == ItemColor{0, 128, 0} || color == ItemColor{0, 0, 128} ||
               color == ItemColor{128, 128, 0};
    }

    [[nodiscard]] constexpr ItemColor normalizedItemColor(ItemColor color) noexcept
    {
        return isSupportedItemColor(color) ? color : ItemColor{};
    }

    struct ItemInstance
    {
        ItemId definition;
        ItemColor color;

        auto operator<=>(const ItemInstance&) const = default;
    };

    struct AttributeBonuses
    {
        int armour{};
        int maximumHealth{};
        int damageMinimum{};
        int damageMaximum{};
        int initiative{};
        int criticalHitChance{};
        int dodge{};
        int precision{};
        int healing{};
        int sleepResistance{};
        int confusionResistance{};
        int debuffResistance{};
        int decayResistance{};
    };

    struct CurrencyItem
    {
        Currency currency{};
    };

    struct LoneVictoryEffect
    {
        int otherPartyHealthThreshold{};
        int damage{};
    };

    using EquipmentBattleEffect = std::variant<LoneVictoryEffect>;

    struct EquippedBattleEffect
    {
        ItemId source;
        EquipmentBattleEffect effect;
    };

    struct EquipmentItem
    {
        AttributeBonuses bonuses;
        bool addsMissOnHighDamage{};
        bool healsOnPass{};
        std::optional<EquipmentBattleEffect> battleEffect;
    };

    struct ConsumableItem
    {
        int healing{};
        AttributeBonuses temporaryBonuses;
        int duration{};
    };

    using ItemProperties = std::variant<CurrencyItem, EquipmentItem, ConsumableItem>;

    struct ItemDefinition
    {
        ItemId id;
        std::string name;
        ItemCategory category{};
        int level{1};
        int price{};
        ItemProperties properties;
        AssetId icon;
        std::vector<std::string> campaigns;
        std::string description;
    };
} // namespace soma::gameplay
