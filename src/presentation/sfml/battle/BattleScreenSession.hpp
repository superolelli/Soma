#pragma once

#include "content/battle/BattleContent.hpp"
#include "content/economy/EconomyContent.hpp"
#include "content/flow/GameFlowContent.hpp"
#include "content/rules/RuleContent.hpp"
#include "game/CampaignState.hpp"
#include "presentation/sfml/app/BattleLaunchOptions.hpp"
#include "presentation/sfml/battle/BattleScreenView.hpp"
#include "presentation/sfml/common/ParticleSystem.hpp"
#include "simulation/battle/BattleSimulation.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace soma::presentation
{
    class IVideoPlayer;
    class IAnimationPlayer;

    struct BattleTransitionRequest
    {
        std::filesystem::path model;
        std::string entity;
        std::string animation;
        sf::Vector2f position;
        float scale{0.7F};
        float delayAfterSeconds{1.0F};
    };

    struct BattleBackgroundLayer
    {
        std::filesystem::path texture;
        sf::Vector2f position;
    };

    struct BattleScreenRequest
    {
        content::BattleContent::Encounter encounter;
        std::vector<gameplay::ContentDefinitionId> enemies;
        std::filesystem::path background;
        std::vector<BattleBackgroundLayer> backgroundLayers;
        std::vector<ParticleEmitterConfig> particles;
        std::filesystem::path music;
        std::uint64_t randomSeed{42};
        std::unordered_map<gameplay::ContentDefinitionId, int> playerHealth;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerFatigue;
        std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> playerBuffs;
        std::optional<gameplay::ContentDefinitionId> bossCombatant;
        double bossHealthAndDamageMultiplier{1.0};
        std::optional<BattleTransitionRequest> transition;
        IAnimationPlayer* preparedTransitionAnimation{};
        IAnimationPlayer* preparedEffectAnimation{};
        const BattleTextures* preparedTextures{};
        std::optional<std::unordered_map<gameplay::EntityId, CombatantView>> preparedCombatantViews;
        std::optional<std::filesystem::path> introVideo;
        std::function<void()> onBattleTransition;
        std::function<void()> onBattleReady;
        const content::GameFlowContent* flow{};
        std::optional<BattleLaunchOptions> developmentLaunchOptions;
        bool returnWhenResolved{};
    };

    enum class BattleScreenResult
    {
        playersWon,
        enemiesWon,
        saveAndMainRoom,
        abandonAndMainRoom,
        windowClosed
    };

    struct BattleScreenOutcome
    {
        BattleScreenResult result{};
        std::unordered_map<gameplay::ContentDefinitionId, int> playerHealth;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerFatigue;
        std::unordered_set<gameplay::AbilityId> usedPlayerAbilities;
        std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> playerBuffs;
    };

    [[nodiscard]] std::vector<simulation::BattleSpawn> createBattleSpawns(
        const content::BattleContent::Encounter& encounter,
        const std::vector<gameplay::ContentDefinitionId>& enemies,
        const std::unordered_map<gameplay::ContentDefinitionId, int>& playerHealth,
        const std::unordered_map<gameplay::ContentDefinitionId, int>& playerFatigue,
        const std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>>& playerBuffs,
        const std::optional<gameplay::ContentDefinitionId>& bossCombatant = std::nullopt,
        double bossHealthAndDamageMultiplier = 1.0);

    [[nodiscard]] BattleScreenOutcome runBattleScreenSession(sf::RenderWindow& window,
                                                             const content::BattleContent& battleContent,
                                                             const content::EconomyContent& economyContent,
                                                             const content::RuleContent& ruleContent,
                                                             game::CampaignState& campaignState,
                                                             BattleScreenRequest request,
                                                             IVideoPlayer* videoPlayer = nullptr);
} // namespace soma::presentation
