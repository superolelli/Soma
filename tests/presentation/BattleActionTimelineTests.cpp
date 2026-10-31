#include "gameplay/random/IRandom.hpp"
#include "presentation/sfml/battle/BattleActionTimeline.hpp"
#include "simulation/battle/BattleSimulation.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <source_location>
#include <string>
#include <vector>

namespace
{
    const soma::gameplay::ContentDefinitionId playerDefinition{"player.test"};
    const soma::gameplay::ContentDefinitionId enemyDefinition{"enemy.test"};
    const soma::gameplay::AbilityId abilityId{"ability.test"};
    const soma::gameplay::EntityId player{"player.test.slot0"};
    const soma::gameplay::EntityId enemy{"enemy.test.slot4"};

    void check(bool condition, const std::source_location& location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    class FixedRandom final : public soma::gameplay::IRandom
    {
      public:
        [[nodiscard]] std::uint64_t seed() const noexcept override { return 1; }
        int between(int minimumInclusive, int) override { return minimumInclusive; }
        bool chance(double probability) override { return probability > 0.0; }
    };

    class FakeAnimation final : public soma::presentation::IAnimationPlayer
    {
      public:
        void play(std::string_view animation, float = 1.0F) override
        {
            current = animation;
            playing = true;
            played.push_back(current);
        }
        void setPosition(float x, float y) override { position = {x, y}; }
        void setScale(float x, float y) override { scale = {x, y}; }
        void update(float) override {}
        void render() override {}
        [[nodiscard]] soma::presentation::AnimationBounds bounds() override { return {position.x, position.y, 80.0F, 120.0F}; }
        [[nodiscard]] std::optional<soma::presentation::AnimationPoint> point(std::string_view) override { return std::nullopt; }
        [[nodiscard]] bool isPlaying() const override { return playing; }
        [[nodiscard]] std::string currentAnimation() const override { return current; }

        bool playing{};
        std::string current{"idle"};
        std::vector<std::string> played;
        sf::Vector2f position;
        sf::Vector2f scale;
    };

    class FakeSounds final : public soma::presentation::IBattleSoundPlayer
    {
      public:
        void playActionStart(const std::vector<soma::gameplay::GameplayEvent>& events) override
        {
            ++startCalls;
            lastStartEventCount = events.size();
        }
        void playResults(const std::vector<soma::gameplay::GameplayEvent>& events) override
        {
            ++resultCalls;
            lastResultEventCount = events.size();
        }

        int startCalls{};
        int resultCalls{};
        std::size_t lastStartEventCount{};
        std::size_t lastResultEventCount{};
    };

    soma::content::BattleContent makeContent()
    {
        soma::content::BattleContent content;
        content.layout.announcementDuration = 0.2;
        content.layout.executionTimeout = 0.5;
        content.layout.reactionDuration = 0.2;
        content.layout.resultDuration = 0.2;
        content.layout.wakeResultDuration = 0.4;
        content.layout.defeatDuration = 0.3;
        content.layout.itemSpeechDuration = 0.3;

        soma::content::AbilityDefinition ability;
        ability.id = abilityId;
        ability.name = "Test";
        ability.animation = "attack";
        ability.hostileEffectAnimation = "impact";
        content.abilities.emplace(ability.id, ability);

        soma::content::CombatantDefinition playerContent;
        playerContent.id = playerDefinition;
        playerContent.name = "Player";
        playerContent.team = soma::gameplay::Team::players;
        playerContent.attributes.maximumHealth = 20;
        playerContent.attributes.initiative = 10;
        playerContent.abilities.push_back(abilityId);
        content.combatants.emplace(playerContent.id, playerContent);

        soma::content::CombatantDefinition enemyContent;
        enemyContent.id = enemyDefinition;
        enemyContent.name = "Enemy";
        enemyContent.team = soma::gameplay::Team::enemies;
        enemyContent.attributes.maximumHealth = 20;
        enemyContent.attributes.initiative = 5;
        enemyContent.abilities.push_back(abilityId);
        content.combatants.emplace(enemyContent.id, enemyContent);

        content.itemActivations.emplace(soma::gameplay::ItemId{"item.test"},
                                        soma::content::ItemActivationPresentation{"Speech", "Item activated"});
        return content;
    }

    soma::presentation::CombatantView makeView(const soma::gameplay::ContentDefinitionId& definition,
                                                std::unique_ptr<FakeAnimation> animation)
    {
        return {std::move(animation), definition, {100.0F, 300.0F}, {{100.0F, 180.0F}, {80.0F, 120.0F}}, 0.6F, 0.0F,
                20.0F, 20.0F, 20, 0.7F, 20, 0.0F, 0.0F, 0, 0.7F};
    }

    struct Fixture
    {
        Fixture()
            : battle(content, {{playerDefinition, 0}, {enemyDefinition, 4}}, random)
            , timeline(sounds)
        {
            auto playerAnimation = std::make_unique<FakeAnimation>();
            playerView = playerAnimation.get();
            views.emplace(player, makeView(playerDefinition, std::move(playerAnimation)));
            auto enemyAnimation = std::make_unique<FakeAnimation>();
            enemyView = enemyAnimation.get();
            views.emplace(enemy, makeView(enemyDefinition, std::move(enemyAnimation)));
        }

        [[nodiscard]] soma::presentation::PresentedAction actionAt(soma::presentation::ActionStage stage) const
        {
            return {{player, abilityId, enemy}, {}, {}, {enemy}, {}, battle.combatants(), {}, stage};
        }

        soma::content::BattleContent content{makeContent()};
        FixedRandom random;
        soma::simulation::BattleSimulation battle;
        FakeSounds sounds;
        soma::presentation::BattleActionTimeline timeline;
        soma::presentation::CombatantViews views;
        FakeAnimation effect;
        FakeAnimation* playerView{};
        FakeAnimation* enemyView{};
        std::vector<soma::presentation::FloatingText> floating;
    };

    void animationCompletionAndTimeoutBothAdvanceExecution()
    {
        for (const auto timeoutPath : {false, true})
        {
            Fixture fixture;
            std::optional action{fixture.actionAt(soma::presentation::ActionStage::execution)};
            action->events.push_back(soma::gameplay::DamageApplied{player, enemy, 4});
            fixture.playerView->playing = timeoutPath;
            fixture.effect.playing = timeoutPath;

            fixture.timeline.update(timeoutPath ? 0.5F : 0.1F, action, fixture.views, fixture.effect, fixture.battle, fixture.floating);

            check(action.has_value());
            check(action->stage == soma::presentation::ActionStage::result);
            check(fixture.sounds.startCalls == 1);
            check(fixture.sounds.resultCalls == 1);
            check(fixture.views.at(enemy).targetHealth == 16);
            fixture.timeline.update(0.2F, action, fixture.views, fixture.effect, fixture.battle, fixture.floating);
            check(!action.has_value());
        }
    }

    void announcementStartsTheAbilityPresentation()
    {
        Fixture fixture;
        std::optional action{fixture.actionAt(soma::presentation::ActionStage::announcement)};

        fixture.timeline.update(0.2F, action, fixture.views, fixture.effect, fixture.battle, fixture.floating);

        check(action->stage == soma::presentation::ActionStage::execution);
        check(fixture.playerView->current == "attack");
        check(fixture.enemyView->current == "attacked");
        check(fixture.effect.current == "impact");
    }

    void itemAndRiposteInterruptionsHaveTheirOwnPresentationStages()
    {
        Fixture itemFixture;
        std::optional itemAction{itemFixture.actionAt(soma::presentation::ActionStage::result)};
        itemAction->deferredEvents = {soma::gameplay::ItemActivated{soma::gameplay::ItemId{"item.test"}, player},
                                      soma::gameplay::DamageApplied{player, enemy, 2}};
        itemFixture.timeline.update(0.2F, itemAction, itemFixture.views, itemFixture.effect, itemFixture.battle, itemFixture.floating);
        check(itemAction->stage == soma::presentation::ActionStage::itemInterruption);
        itemFixture.timeline.update(0.3F, itemAction, itemFixture.views, itemFixture.effect, itemFixture.battle, itemFixture.floating);
        check(itemAction->stage == soma::presentation::ActionStage::result);
        check(itemFixture.floating.back().value == "2");
        check(std::ranges::any_of(itemFixture.floating, [](const auto& text) { return text.value == "Item activated"; }));

        Fixture riposteFixture;
        std::optional riposteAction{riposteFixture.actionAt(soma::presentation::ActionStage::result)};
        riposteAction->deferredEvents = {soma::gameplay::StatusTriggered{enemy, soma::gameplay::StatusId{"status.riposte"}},
                                         soma::gameplay::DamageApplied{enemy, player, 3}};
        riposteFixture.timeline.update(
            0.2F, riposteAction, riposteFixture.views, riposteFixture.effect, riposteFixture.battle, riposteFixture.floating);
        check(riposteAction->stage == soma::presentation::ActionStage::reaction);
        check(riposteFixture.playerView->current == "attacked");
        check(riposteFixture.floating.front().value == "Gegenschlag!");
        riposteFixture.timeline.update(
            0.2F, riposteAction, riposteFixture.views, riposteFixture.effect, riposteFixture.battle, riposteFixture.floating);
        check(riposteAction->stage == soma::presentation::ActionStage::result);
    }

    void defeatFinishesBeforeDeferredOutcomesAndEventuallyResets()
    {
        Fixture fixture;
        std::optional action{fixture.actionAt(soma::presentation::ActionStage::result)};
        action->events = {soma::gameplay::CombatantDefeated{enemy, player}};
        action->deferredEvents = {soma::gameplay::StatusGained{player, soma::gameplay::StatusId{"status.marked"}}};

        fixture.timeline.update(0.2F, action, fixture.views, fixture.effect, fixture.battle, fixture.floating);
        check(action->stage == soma::presentation::ActionStage::defeat);
        check(fixture.enemyView->current == "dying");
        fixture.timeline.update(0.3F, action, fixture.views, fixture.effect, fixture.battle, fixture.floating);
        check(action->stage == soma::presentation::ActionStage::result);
        check(fixture.floating.back().value == "Markiert!");
        fixture.timeline.update(0.2F, action, fixture.views, fixture.effect, fixture.battle, fixture.floating);
        check(!action.has_value());
    }
} // namespace

int main()
{
    animationCompletionAndTimeoutBothAdvanceExecution();
    announcementStartsTheAbilityPresentation();
    itemAndRiposteInterruptionsHaveTheirOwnPresentationStages();
    defeatFinishesBeforeDeferredOutcomesAndEventuallyResets();
}
