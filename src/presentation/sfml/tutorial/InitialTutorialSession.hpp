#pragma once

#include "content/battle/BattleContent.hpp"
#include "content/economy/EconomyContent.hpp"
#include "content/flow/GameFlowContent.hpp"
#include "content/rules/RuleContent.hpp"
#include "content/tutorial/TutorialContent.hpp"
#include "content/world/WorldContent.hpp"
#include "game/CampaignState.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include <vector>

namespace soma::presentation
{
    enum class InitialTutorialResult
    {
        completed,
        windowClosed
    };

    /// Runs the fixed introductory corridor, loot, battle, rewards, and main-room entrance sequence.
    class InitialTutorialSession
    {
      public:
        InitialTutorialSession(sf::RenderWindow& window,
                               const content::BattleContent& battle,
                               const content::EconomyContent& economy,
                               const content::WorldContent& world,
                               const content::GameFlowContent& flow,
                               const content::RuleContent& rules,
                               const content::TutorialContent& tutorial,
                               game::CampaignState& campaign);

        [[nodiscard]] InitialTutorialResult run();

      private:
        void createParty();
        void update(sf::Time elapsed, float direction);
        void drawFrame();
        void drawChest();
        void drawDoor();
        [[nodiscard]] bool showMessage(std::string_view id, std::vector<content::Rectangle> highlights = {});
        [[nodiscard]] bool showMessages(const std::vector<std::string>& ids);
        [[nodiscard]] bool openChest();
        [[nodiscard]] bool runBattle();
        [[nodiscard]] bool showBattleRewards();
        [[nodiscard]] bool playEnding();
        [[nodiscard]] bool showDialogue(const content::TutorialDialogueLine& line);
        void animatePlayerEntrance(std::size_t playerIndex);
        [[nodiscard]] float movementDirection() const;
        [[nodiscard]] sf::FloatRect chestBounds() const;

        sf::RenderWindow& window_;
        const content::BattleContent& battle_;
        const content::EconomyContent& economy_;
        const content::WorldContent& world_;
        const content::GameFlowContent& flow_;
        const content::RuleContent& rules_;
        const content::TutorialContent& tutorial_;
        game::CampaignState& campaign_;
        sf::View worldView_;
        sf::View hudView_;
        std::array<sf::Texture, 3> backgrounds_;
        sf::Texture chestTexture_;
        sf::Texture doorTexture_;
        sf::Texture speechBubbleTexture_;
        sf::Font font_;
        std::vector<std::unique_ptr<class SpriterAnimationPlayer>> party_;
        std::array<bool, 4> entered_{};
        float viewLeft_{};
        float doorOffset_{};
        bool walking_{};
        bool chestOpened_{};
        bool battleCompleted_{};
        bool lootHintShown_{};
    };
} // namespace soma::presentation
