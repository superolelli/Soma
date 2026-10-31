#pragma once

#include "content/battle/BattleContent.hpp"
#include "presentation/sfml/battle/BattleActionPresentation.hpp"
#include "presentation/sfml/battle/BattleInteractionController.hpp"
#include "presentation/sfml/battle/BattleSceneBlur.hpp"
#include "presentation/sfml/common/ParticleSystem.hpp"
#include "presentation/sfml/common/ResourceCounter.hpp"
#include "simulation/battle/BattleSimulation.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

namespace soma::presentation
{
    /// Draws the battle scene, combatants, overlays, tooltips, and staged action foreground.
    class BattleScreenRenderer
    {
      public:
        BattleScreenRenderer(sf::RenderWindow& window,
                             const content::BattleContent& content,
                             const simulation::BattleSimulation& battle,
                             CombatantViews& views,
                             IAnimationPlayer& effect,
                             const BattleTextures& textures,
                             const sf::Font& interfaceFont,
                             const sf::Font& notificationFont,
                             const std::vector<sf::Sprite>& backgrounds,
                             ParticleSystem& particles,
                             BattleSceneBlur& sceneBlur,
                             const std::optional<PresentedAction>& action,
                             const std::vector<FloatingText>& floatingTexts,
                             const BattleInteractionController& interaction,
                             const ResourceCounter& resources,
                             sf::Vector2f skipTurnPosition,
                             sf::FloatRect skipTurnBounds,
                             unsigned int skipTurnFrameWidth);

        void render(sf::Vector2f mousePosition);
        void draw(sf::Vector2f mousePosition);

      private:
        [[nodiscard]] bool isExecutingAbility() const;
        [[nodiscard]] bool participatesInAction(const gameplay::EntityId& id) const;
        [[nodiscard]] bool awaitsDefeatAnimation(const gameplay::EntityId& id) const;
        [[nodiscard]] std::vector<gameplay::EntityId> targetsMarkedBy(const gameplay::AbilityId& ability,
                                                                      const gameplay::EntityId& primaryTarget) const;

        void drawRestingCombatants();
        void drawExecutingAction();
        void drawTurnOrderNumber(const gameplay::EntityId& combatant,
                                 const CombatantView& view,
                                 const std::vector<gameplay::EntityId>& initiativeOrder);
        [[nodiscard]] std::optional<std::pair<std::string, sf::Vector2f>> drawCombatantOverlays(sf::Vector2f mousePosition,
                                                                                                bool skipActionParticipants = false);
        void drawPlayerControls(sf::Vector2f mousePosition);
        void drawHoveredTargetMarkers(sf::Vector2f mousePosition);
        void drawAbilityTooltip(sf::Vector2f mousePosition);
        void drawAnnouncement();
        void drawItemInterruption();
        void drawInformationPanel();
        void drawInformationStatTooltip(sf::Vector2f mousePosition);
        void drawFloatingTexts();
        void drawBattleResult();
        void drawTargetMarker(const gameplay::EntityId& target);
        [[nodiscard]] simulation::CombatantState displayedState(const simulation::CombatantState& state) const;

        sf::RenderWindow& window_;
        const content::BattleContent& content_;
        const simulation::BattleSimulation& battle_;
        CombatantViews& views_;
        IAnimationPlayer& effect_;
        const BattleTextures& textures_;
        const sf::Font& interfaceFont_;
        const sf::Font& notificationFont_;
        const std::vector<sf::Sprite>& backgrounds_;
        ParticleSystem& particles_;
        BattleSceneBlur& sceneBlur_;
        const std::optional<PresentedAction>& action_;
        const std::vector<FloatingText>& floatingTexts_;
        const BattleInteractionController& interaction_;
        const ResourceCounter& resources_;
        sf::Vector2f skipTurnPosition_;
        sf::FloatRect skipTurnBounds_;
        unsigned int skipTurnFrameWidth_;
    };
} // namespace soma::presentation
