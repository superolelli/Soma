#pragma once

#include "content/EconomyContent.hpp"
#include "content/ParticleDefinition.hpp"

#include <unordered_map>

namespace soma::content
{
    struct EncounterDefinition
    {
        gameplay::EncounterId id;
        std::vector<gameplay::ContentDefinitionId> combatants;
        int budgetCost{};
        bool boss{};
        std::optional<gameplay::ContentDefinitionId> bossCombatant;
        std::optional<gameplay::AssetId> introCue;
        gameplay::LootTableId rewardTable;
    };

    struct LootEntryDefinition
    {
        gameplay::ItemId item;
        int weight{1};
    };

    struct LootTableDefinition
    {
        gameplay::LootTableId id;
        std::vector<LootEntryDefinition> entries;
        int minimumRolls{1};
        int maximumRolls{1};
        double additionalRollChance{};
    };

    struct LootableDefinition
    {
        gameplay::LootableId id;
        std::string name;
        std::vector<std::string> campaigns;
        gameplay::LootTableId lootTable;
        gameplay::AssetId sprite;
        gameplay::AssetId sound;
        Rectangle hitbox;
        Point position;
    };

    struct LevelDefinition
    {
        gameplay::LevelId id;
        std::string campaign;
        int difficulty{};
        int roomCount{};
        int minimumEncounterBudget{};
        int maximumEncounterBudget{};
        double lootableProbability{};
        std::vector<gameplay::AssetId> roomBackgrounds;
        gameplay::AssetId endBackground;
        std::vector<gameplay::EncounterId> encounters;
        gameplay::EncounterId bossEncounter;
    };

    struct WorldContent
    {
        struct CampaignPresentation
        {
            std::vector<gameplay::AssetId> secondLayers;
            std::vector<gameplay::AssetId> thirdLayers;
            gameplay::AssetId fourthLayer;
            std::vector<gameplay::AssetId> musicTracks;
            int secondLayerDivisor{3};
            int thirdLayerDivisor{6};
            int fourthLayerDivisor{9};
            double musicVolume{50.0};
            double battleMusicVolume{30.0};
            double battleCrossfadeSeconds{4.0};
            std::vector<ParticleEmitterDefinition> particles;
        };

        std::unordered_map<gameplay::EncounterId, EncounterDefinition> encounters;
        std::unordered_map<gameplay::LootTableId, LootTableDefinition> lootTables;
        std::unordered_map<gameplay::LootableId, LootableDefinition> lootables;
        std::unordered_map<gameplay::LevelId, LevelDefinition> levels;
        std::unordered_map<gameplay::AssetId, std::filesystem::path> assets;
        std::unordered_map<std::string, CampaignPresentation> campaignPresentation;

        struct Presentation
        {
            struct LootDialog
            {
                Point position;
                Point titleCenter;
                Point firstItem;
                Point itemSpacing;
                Point closeButton;
                Point takeAllButton;
                Point itemSize;
                Point quantityOffset;
                Point errorCenter;
                int columns{3};
            } lootDialog;

            struct LevelResultDialog
            {
                Point title;
                Point rewardDescription;
                Point rewardCards;
                Point rewardDice;
                Point totalCards;
                Point totalDice;
                Point continueButton;
                Point unlockTitle;
                Point firstUnlockedItem;
                Point unlockedItemSpacing;
            } levelResultDialog;

            struct BattleTransition
            {
                gameplay::AssetId model;
                std::string entity;
                std::string animation;
                Point position;
                double scale{0.7};
                double delayAfterSeconds{1.0};
            } battleTransition;

            struct SpeechBubbles
            {
                gameplay::AssetId texture;
                std::unordered_map<gameplay::ContentDefinitionId, std::vector<std::string>> lines;
                double checkIntervalSeconds{1.0};
                double appearanceChance{0.1};
                double durationSeconds{3.0};
                int maximumTextWidth{200};
                int characterSize{18};
                Point padding{20, 20};
                Point speakerOffset{-10, -10};
                int textVerticalOffset{-17};
            } speechBubbles;

            Point referenceSize{1920, 1080};
            std::array<Point, 4> partyPositions;
            double movementSpeed{220.0};
            double partyScale{0.6};
            std::string idleAnimation{"idle"};
            std::string walkAnimation{"walk"};
            double idleAnimationSpeed{1.0};
            double walkAnimationSpeed{0.85};

            struct PartyStatus
            {
                gameplay::AssetId healthBar;
                gameplay::AssetId healthBarFrame;
                gameplay::AssetId fatigueBar;
                gameplay::AssetId fatigueBarFrame;
                gameplay::AssetId fatigueTired;
                gameplay::AssetId fatigueStupid;
                gameplay::AssetId buff;
                int verticalOffset{30};
                int statusGap{2};
            } partyStatus;
        } presentation;
    };
} // namespace soma::content
