#pragma once

#include "game/ExplorationConsumableUseService.hpp"
#include "game/RuleBuilderService.hpp"
#include "gameplay/random/SeededRandom.hpp"
#include "presentation/sfml/ActiveRulesPanel.hpp"
#include "presentation/sfml/BattleActionPresentation.hpp"
#include "presentation/sfml/BattleScreenSession.hpp"
#include "presentation/sfml/BattleScreenView.hpp"
#include "presentation/sfml/EconomySoundPlayer.hpp"
#include "presentation/sfml/EconomyTextures.hpp"
#include "presentation/sfml/ExplorationConsumablePanel.hpp"
#include "presentation/sfml/ExplorationHud.hpp"
#include "presentation/sfml/ExplorationMusicPlayer.hpp"
#include "presentation/sfml/ExplorationPartyStatus.hpp"
#include "presentation/sfml/ExplorationSession.hpp"
#include "presentation/sfml/ExplorationSpeechBubbles.hpp"
#include "presentation/sfml/LevelResultDialog.hpp"
#include "presentation/sfml/LootDialog.hpp"
#include "presentation/sfml/PauseMenu.hpp"
#include "presentation/sfml/SfeMovieVideoPlayer.hpp"
#include "presentation/sfml/SpriterAnimationPlayer.hpp"
#include "simulation/rewards/LevelRewardCalculator.hpp"
#include "simulation/rewards/LootGenerator.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace soma::presentation::detail
{
    /// Owns the runtime state of one exploration level and coordinates traversal, encounters, rewards, persistence, and presentation.
    class ExplorationSession
    {
      public:
        ExplorationSession(const content::BattleContent& battle,
                           const content::EconomyContent& economy,
                           const content::WorldContent& world,
                           const content::RuleContent& rules,
                           gameplay::LevelId level,
                           game::CampaignState& campaignState,
                           game::EconomyService& economyService,
                           std::uint64_t randomSeed,
                           std::optional<simulation::LevelRun> authoredLevel,
                           sf::RenderWindow* sharedWindow = nullptr,
                           const content::GameFlowContent* flow = nullptr,
                           std::function<void()> loadingFrame = {},
                           std::optional<game::ActiveLevelState> savedLevel = std::nullopt);

        ExplorationResult run();
        [[nodiscard]] game::ActiveLevelState activeLevelState() const;

      private:
        static const std::array<gameplay::ContentDefinitionId, 4>& playerDefinitions();
        static gameplay::ContentDefinitionId battleEncounterFor(const content::LevelDefinition& level);
        static std::vector<std::filesystem::path> campaignMusicTracks(const content::WorldContent& world, const content::LevelDefinition& level);
        static std::set<content::RuleFragmentType> discoveredRuleTypes(const game::CampaignState& state, const content::RuleContent& rules);
        static std::set<content::RuleFragmentType> discoveredRuleTypes(const game::CampaignStateData& state, const content::RuleContent& rules);

        void showLoadingFrame();
        void rememberLevelBeforePendingEncounter();
        void requestSessionClose();
        void initializePlayerHealth();
        void initializePlayerFatigue();
        void loadWorldTextures();
        void loadCursors();
        void createParty();

        bool processEvents();
        void handleConsumableRelease(sf::Vector2i pixel);
        bool handleKey(const sf::Event::KeyPressed& key);
        void handleClick(sf::Vector2i pixel);
        void tryOpenLootable(sf::Vector2f mouse);
        void updateCursor();
        [[nodiscard]] bool isHoveringLootable(sf::Vector2f mouse) const;
        [[nodiscard]] sf::FloatRect lootableHitbox(std::size_t roomIndex) const;
        bool tryOpenLootable(std::size_t roomIndex, sf::Vector2f mouse);

        bool runBattle();
        void prepareUpcomingBattleViews();
        void appendBattleParallaxLayers(BattleScreenRequest& request) const;
        void appendBattleParallaxLayer(BattleScreenRequest& request, std::span<const gameplay::AssetId> tiles, int scrollDivisor) const;
        void applyBattleOutcome(const content::EncounterDefinition& encounter,
                                const content::LevelDefinition& level,
                                const BattleScreenOutcome& outcome);
        void openBattleRewards(const content::EncounterDefinition& encounter, const content::LevelDefinition& level);
        void openLevelResult(const content::LevelDefinition& level, bool completed);
        [[nodiscard]] std::vector<gameplay::ItemId> newlyUnlockedItems() const;
        [[nodiscard]] std::vector<content::RuleFragmentType> newlyDiscoveredRuleTypes() const;
        void grantLevelRewards(const simulation::LevelRewards& rewards);

        void update(sf::Time elapsed);
        void updatePartyStatus(sf::Time elapsed);
        void updateSpeechBubbles(sf::Time elapsed);
        [[nodiscard]] std::array<AnimationBounds, 4> partyBounds();
        [[nodiscard]] std::array<bool, 4> livingPartyMembers() const;
        [[nodiscard]] int effectiveMaximumHealth(const gameplay::ContentDefinitionId& id) const;
        [[nodiscard]] float movementDirection() const;
        void enterVisibleRoom(float viewLeft);
        void updateParty(sf::Time elapsed, float direction, float viewLeft);
        void render();
        void drawExplorationFrame();
        void renderParallax();
        void renderParallaxLayer(std::span<const gameplay::AssetId> tiles, int scrollDivisor);
        void renderRooms();
        void renderParty();
        void renderPartyStatus();
        void renderHud();

        const content::BattleContent& battle_;
        const content::EconomyContent& economy_;
        const content::WorldContent& world_;
        const content::RuleContent& rules_;
        gameplay::LevelId levelId_;
        game::CampaignState& campaignState_;
        game::EconomyService& economyService_;
        game::RuleBuilderService ruleService_;
        game::CampaignStateData baselineCampaign_;
        std::set<gameplay::ItemId> initiallyUnlockedItems_;
        std::set<content::RuleFragmentType> initiallyDiscoveredRuleTypes_;
        content::BattleContent campaignBattle_;
        gameplay::SeededRandom random_;
        gameplay::SeededRandom speechRandom_;
        simulation::LevelRun levelRun_;
        ExplorationMusicPlayer music_;
        SfeMovieVideoPlayer videoPlayer_;
        const content::WorldContent::Presentation& layout_;
        std::unique_ptr<sf::RenderWindow> ownedWindow_;
        sf::RenderWindow& window_;
        bool embedded_{};
        std::function<void()> loadingFrame_;
        const content::GameFlowContent* flow_{};
        bool returnRequested_{};
        std::unique_ptr<PauseMenu> pauseMenu_;
        sf::View view_;
        sf::View hudView_;
        sf::Font font_;
        EconomySoundPlayer sounds_;
        EconomyTextures economyTextures_;
        BattleTextures battleTextures_;
        std::unordered_map<gameplay::AssetId, sf::Texture> textures_;
        float roomWidth_{};
        float maximumViewLeft_{};
        std::vector<std::unique_ptr<SpriterAnimationPlayer>> party_;
        std::unique_ptr<SpriterAnimationPlayer> battleTransition_;
        std::unique_ptr<SpriterAnimationPlayer> battleEffect_;
        CombatantViews preparedBattleViews_;
        std::optional<gameplay::EncounterId> preparedEncounter_;
        std::unique_ptr<ExplorationSpeechBubbles> speechBubbles_;
        std::unique_ptr<ExplorationPartyStatus> partyStatus_;
        std::unique_ptr<ExplorationHud> explorationHud_;
        std::unique_ptr<ExplorationConsumablePanel> explorationConsumables_;
        std::unique_ptr<ActiveRulesPanel> activeRules_;
        std::optional<sf::Image> arrowCursorImage_;
        std::optional<sf::Image> handCursorImage_;
        std::optional<sf::Cursor> arrowCursor_;
        std::optional<sf::Cursor> handCursor_;
        sf::Shader lootableOutlineShader_;
        bool lootableOutlineAvailable_{};
        sf::Clock clock_;
        bool walking_{};
        std::optional<gameplay::EncounterId> pendingEncounter_;
        std::optional<simulation::LevelRun> levelRunBeforeBattle_;
        std::unique_ptr<LootDialog> lootDialog_;
        std::unique_ptr<LevelResultDialog> levelResultDialog_;
        bool completeLevelAfterLoot_{};
        simulation::LootGenerator lootGenerator_;
        game::ExplorationConsumableUseService explorationConsumableUse_;
        bool defeated_{};
        ExplorationResult result_{ExplorationResult::abandoned};
        int enemiesDefeated_{};
        std::unordered_set<gameplay::AbilityId> usedPlayerAbilities_;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerHealth_;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerFatigue_;
        std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> playerBuffs_;
    };
} // namespace soma::presentation::detail
