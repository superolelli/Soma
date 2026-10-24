#pragma once

#include "content/BattleContent.hpp"
#include "gameplay/items/ItemTypes.hpp"

#include <filesystem>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

namespace soma::content
{
    struct Point
    {
        int x{};
        int y{};

        auto operator<=>(const Point&) const = default;
    };

    struct Rectangle
    {
        int x{};
        int y{};
        int width{};
        int height{};
    };

    struct InventoryLayout
    {
        Point origin;
        Point itemsPanel;
        Point equipmentPanel;
        Point attributes;
        Point previousButton;
        Point nextButton;
        Point closeButton;
        std::array<Point, 4> equipmentFields;
        Point equipmentItemInset;
        Point equipmentBackground;
        Point equipmentDiamond;
        std::array<Point, 4> equipmentConnections;
        Rectangle itemsScrollbar;
    };

    struct ShopLayout
    {
        Point vendingOrigin;
        Point inventoryPanel;
        Point consumablesRow;
        Point shopPanel;
        Point shopItemOrigin;
        Point shopItemSpacing;
        Point consumableOrigin;
        Point consumableSpacing;
        Point dice;
        Point dicePriceLock;
        Point buyButton;
        Point closeButton;
        Point title;
        Point offerTitle;
        Point selectionInset;
        Point priceSignOffset;
        Point slotLockOffset;
        Rectangle slotUnlockPrice;
        Point reducedPriceSignOffset;
        Rectangle diceUnlockPrice;
        Point actionPrice;
        Point actionCards;
        Point confirmationMessage;
        int confirmationMessageWidth{};
        Point confirmationYesButton;
        Point confirmationNoButton;
        Point sellSlider;
        Point sellConfirmButton;
        Point sellCancelButton;
        Rectangle sellQuantityBounds;
        Point ownedItemInset;
        Point ownedItemSpacing;
        Point itemSize;
        Point quantityOffset;
    };

    struct SkillLayout
    {
        Point origin;
        Rectangle playerNameBounds;
        Rectangle panelTitleBounds;
        std::array<Rectangle, 4> abilityHitboxes;
        Rectangle abilityNameBounds;
        Rectangle abilityIconBounds;
        std::array<Point, 4> abilityBridges;
        std::array<Point, 2> treeOrigins;
        Point versionAbilityIcon;
        std::array<Point, 2> versionSelectors;
        std::array<Point, 5> skillIcons;
        std::array<Point, 4> connectionOffsets;
        std::array<int, 4> connectionTypes;
        Point acquiredInset;
        Point selectionInset;
        Point dice;
        Point buyButton;
        Point previousButton;
        Point nextButton;
        Point closeButton;
        Rectangle selectedSkillTitleBounds;
        Point diceCostOffset;
    };

    struct EconomyPresentation
    {
        Point referenceSize{1920, 1080};
        struct ResourceCounterLayout
        {
            gameplay::AssetId background;
            Point backgroundPosition;
            Point diceIcon;
            Point diceTextCenter;
            Point cardsIcon;
            Point cardsTextCenter;
            double diceScale{1.0};
            double cardsScale{1.0};
            int textSize{};
        } resourceCounter;
        InventoryLayout inventory;
        ShopLayout shop;
        SkillLayout skills;
        struct ConsumableDrawerLayout
        {
            Point openPosition;
            int closedX{};
            Point toggleSize;
            Point toggleInset;
            Point rowInset;
            Point firstItemInset;
            int itemSpacing{};
        } consumableDrawer;
        struct ExplorationConsumableLayout
        {
            Point position;
            Point rowInset;
            Point firstItemInset;
            Point quantityOffset;
            int itemSpacing{};
            int itemSize{};
        } explorationConsumables;
    };

    struct DiamondBonusDefinition
    {
        gameplay::ItemColor color;
        std::optional<gameplay::ItemColor> requiredConnectionColor;
        gameplay::AttributeBonuses bonuses;
    };

    enum class SkillEffectTarget
    {
        both,
        friendly,
        hostile,
        allCombatants
    };

    struct SkillEffectModifier
    {
        SkillEffectTarget target{SkillEffectTarget::both};
        AbilityEffect effect;
    };

    struct AbilityUpgrade
    {
        int additionalTargets{};
        int precisionModifier{};
        int fatigue{};
        std::vector<SkillEffectModifier> effects;
    };

    struct SkillDefinition
    {
        gameplay::SkillId id;
        gameplay::ContentDefinitionId player;
        gameplay::AbilityId ability;
        int slot{};
        std::string name;
        std::string description;
        int cost{};
        std::vector<gameplay::SkillId> prerequisites;
        gameplay::AssetId icon;
        AbilityUpgrade upgrade;
    };

    struct AbilityVersionDefinition
    {
        gameplay::AbilityId ability;
        std::array<gameplay::SkillId, 5> skills;
    };

    struct AbilitySetDefinition
    {
        gameplay::ContentDefinitionId player;
        std::size_t slot{};
        std::array<AbilityVersionDefinition, 2> versions;
    };

    struct EconomyContent
    {
        std::unordered_map<gameplay::ItemId, gameplay::ItemDefinition> items;
        std::unordered_map<gameplay::SkillId, SkillDefinition> skills;
        std::vector<AbilitySetDefinition> abilitySets;
        std::unordered_map<gameplay::AssetId, std::filesystem::path> assets;
        std::vector<DiamondBonusDefinition> diamondBonuses;
        EconomyPresentation presentation;
    };
} // namespace soma::content
