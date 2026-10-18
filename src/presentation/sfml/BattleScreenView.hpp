#pragma once

#include "gameplay/combat/CombatTypes.hpp"
#include "gameplay/events/GameplayEvent.hpp"
#include "presentation/animation/IAnimationPlayer.hpp"
#include "simulation/battle/BattleSimulation.hpp"

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

namespace soma::presentation
{
    struct CombatantView
    {
        std::unique_ptr<IAnimationPlayer> animation;
        gameplay::ContentDefinitionId definition;
        sf::Vector2f normalPosition;
        sf::FloatRect bounds;
        float normalScale{};
        float scaleAdjustment{};
        float displayedHealth{};
        float healthAnimationStart{};
        int targetHealth{};
        float healthAnimationElapsed{0.7F};
        int maximumHealth{};
        float displayedFatigue{};
        float fatigueAnimationStart{};
        int targetFatigue{};
        float fatigueAnimationElapsed{0.7F};
    };

    enum class FloatingTextAnchor
    {
        combatant
    };

    enum class FloatingTextFont
    {
        notification,
        interface
    };

    struct FloatingText
    {
        gameplay::EntityId subject;
        std::string value;
        sf::Color color;
        float elapsed{};
        unsigned int size{30U};
        FloatingTextAnchor anchor{FloatingTextAnchor::combatant};
        sf::Color outlineColor{sf::Color::Black};
        float outlineThickness{3.0F};
        FloatingTextFont font{FloatingTextFont::notification};
    };

    struct AbilityButton
    {
        std::shared_ptr<sf::Texture> texture;
        sf::Sprite sprite;
        sf::FloatRect bounds;
        gameplay::AbilityId ability;

        AbilityButton(std::shared_ptr<sf::Texture> icon, sf::Vector2f position, gameplay::AbilityId abilityId)
            : texture(std::move(icon))
            , sprite(*texture)
            , bounds(position, sf::Vector2f{static_cast<float>(texture->getSize().x), static_cast<float>(texture->getSize().y)})
            , ability(std::move(abilityId))
        {
            sprite.setPosition(position);
        }
    };

    enum class ActionStage
    {
        announcement,
        execution,
        reaction,
        itemInterruption,
        result,
        defeat
    };

    struct PresentedAction
    {
        simulation::BattleActionCommand command;
        std::vector<gameplay::GameplayEvent> events;
        std::vector<gameplay::GameplayEvent> deferredEvents;
        std::vector<gameplay::EntityId> affectedCombatants;
        std::vector<sf::Vector2f> effectPositions;
        std::vector<simulation::CombatantState> combatantsBeforeAction;
        std::optional<gameplay::EntityId> wakingCombatant;
        ActionStage stage{ActionStage::announcement};
        float elapsed{};
        bool startSoundsPlayed{};
    };

    struct BattleTextures
    {
        explicit BattleTextures(const content::BattleContent& content)
            : abilityPanel(content.assets.at("ability_panel"))
            , currentAbilityFrame(content.assets.at("current_ability_frame"))
            , skipTurn(content.assets.at("skip_turn"))
            , combatantInformationPanel(content.assets.at("combatant_information_panel"))
            , healthBar(content.assets.at("health_bar"))
            , healthBarFrame(content.assets.at("health_bar_frame"))
            , healthBarBig(content.assets.at("health_bar_big"))
            , healthBarBigFrame(content.assets.at("health_bar_big_frame"))
            , fatigueBar(content.assets.at("fatigue_bar"))
            , fatigueBarFrame(content.assets.at("fatigue_bar_frame"))
            , fatigueBarBig(content.assets.at("fatigue_bar_big"))
            , fatigueBarBigFrame(content.assets.at("fatigue_bar_big_frame"))
            , targetMarker(content.assets.at("target_marker"))
            , turnMarker(content.assets.at("turn_marker"))
            , announcement(content.assets.at("announcement"))
            , buff(content.assets.at("status_buff"))
            , debuff(content.assets.at("status_debuff"))
            , confused(content.assets.at("status_confused"))
            , marked(content.assets.at("status_marked"))
            , sleeping(content.assets.at("status_sleeping"))
            , missed(content.assets.at("status_missed"))
            , dynamite(content.assets.at("status_dynamite"))
            , bounty(content.assets.at("status_bounty"))
            , decay(content.assets.at("status_decay"))
            , fistOfRevenge(content.assets.at("status_fist_of_revenge"))
            , noface(content.assets.at("status_noface"))
            , fatigueTired(content.assets.at("status_fatigue_tired"))
            , fatigueStupid(content.assets.at("status_fatigue_stupid"))
            , riposte(content.assets.at("status_riposte"))
            , rage(content.assets.at("status_rage"))
            , hacked(content.assets.at("status_hacked"))
            , transfusion(content.assets.at("status_transfusion"))
            , panic(content.assets.at("status_panic"))
            , itemSpeechBubble(content.assets.at("item_speech_bubble"))
        {
        }

        sf::Texture abilityPanel;
        sf::Texture currentAbilityFrame;
        sf::Texture skipTurn;
        sf::Texture combatantInformationPanel;
        sf::Texture healthBar;
        sf::Texture healthBarFrame;
        sf::Texture healthBarBig;
        sf::Texture healthBarBigFrame;
        sf::Texture fatigueBar;
        sf::Texture fatigueBarFrame;
        sf::Texture fatigueBarBig;
        sf::Texture fatigueBarBigFrame;
        sf::Texture targetMarker;
        sf::Texture turnMarker;
        sf::Texture announcement;
        sf::Texture buff;
        sf::Texture debuff;
        sf::Texture confused;
        sf::Texture marked;
        sf::Texture sleeping;
        sf::Texture missed;
        sf::Texture dynamite;
        sf::Texture bounty;
        sf::Texture decay;
        sf::Texture fistOfRevenge;
        sf::Texture noface;
        sf::Texture fatigueTired;
        sf::Texture fatigueStupid;
        sf::Texture riposte;
        sf::Texture rage;
        sf::Texture hacked;
        sf::Texture transfusion;
        sf::Texture panic;
        sf::Texture itemSpeechBubble;
    };
} // namespace soma::presentation
