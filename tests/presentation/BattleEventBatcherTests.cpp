#include "presentation/battle/BattleEventBatcher.hpp"

#include <cstdlib>
#include <iostream>
#include <source_location>
#include <string_view>
#include <vector>

namespace
{
    using soma::gameplay::GameplayEvent;

    const soma::gameplay::EntityId actor{"player.simon"};
    const soma::gameplay::EntityId firstTarget{"enemy.first"};
    const soma::gameplay::EntityId secondTarget{"enemy.second"};

    void check(bool condition, const std::source_location& location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    template <typename Event>
    bool is(const GameplayEvent& event)
    {
        return std::holds_alternative<Event>(event);
    }

    void checkTypes(const std::vector<GameplayEvent>& events, const std::vector<std::size_t>& expectedTypes)
    {
        check(events.size() == expectedTypes.size());
        for (std::size_t index = 0; index < events.size(); ++index)
        {
            check(events[index].index() == expectedTypes[index]);
        }
    }

    template <typename... Events>
    std::vector<std::size_t> typesOf(const Events&... events)
    {
        return {GameplayEvent{events}.index()...};
    }

    void metadataIsDiscardedBeforeProducingABatch()
    {
        std::vector<GameplayEvent> deferred{
            soma::gameplay::TurnStarted{actor},
            soma::gameplay::AbilityUsed{actor, soma::gameplay::AbilityId{"ability.test"}},
            soma::gameplay::TargetRedirected{actor, firstTarget, secondTarget},
            soma::gameplay::DamageApplied{actor, firstTarget, 4}};

        const auto batch = soma::presentation::BattleEventBatcher::takeNext(deferred);

        checkTypes(batch, typesOf(soma::gameplay::DamageApplied{}));
        check(deferred.empty());
    }

    void criticalHitsStayWithTheirDamageOrHealingOutcome()
    {
        struct TestCase
        {
            GameplayEvent outcome;
            GameplayEvent boundary;
        };
        const std::vector<TestCase> cases{
            {soma::gameplay::DamageApplied{actor, firstTarget, 7},
             soma::gameplay::StatusGained{firstTarget, soma::gameplay::StatusId{"status.marked"}}},
            {soma::gameplay::HealingApplied{actor, actor, 5}, soma::gameplay::FatigueChanged{actor, 1, 0, 1}}};

        for (const auto& testCase : cases)
        {
            std::vector<GameplayEvent> deferred{
                soma::gameplay::CriticalHit{actor}, testCase.outcome, soma::gameplay::CriticalHit{secondTarget}, testCase.boundary};

            const auto batch = soma::presentation::BattleEventBatcher::takeNext(deferred);

            check(batch.size() == 3);
            check(is<soma::gameplay::CriticalHit>(batch[0]));
            check(batch[1].index() == testCase.outcome.index());
            check(is<soma::gameplay::CriticalHit>(batch[2]));
            check(deferred.size() == 1);
            check(deferred.front().index() == testCase.boundary.index());
        }
    }

    void riposteTriggerIsPairedOnlyWithItsImmediateOutcome()
    {
        std::vector<GameplayEvent> deferred{
            soma::gameplay::StatusTriggered{actor, soma::gameplay::StatusId{"status.riposte"}},
            soma::gameplay::DamageApplied{actor, firstTarget, 3},
            soma::gameplay::DamageApplied{actor, secondTarget, 4}};

        const auto batch = soma::presentation::BattleEventBatcher::takeNext(deferred);

        checkTypes(batch, typesOf(soma::gameplay::StatusTriggered{}, soma::gameplay::DamageApplied{}));
        check(deferred.size() == 1);
        check(std::get<soma::gameplay::DamageApplied>(deferred.front()).target == secondTarget);
    }

    void itemActivationOwnsOutcomesUntilTheNextPresentationBoundary()
    {
        const soma::gameplay::ItemId firstItem{"item.first"};
        const soma::gameplay::ItemId secondItem{"item.second"};
        std::vector<GameplayEvent> deferred{
            soma::gameplay::ItemActivated{firstItem, actor},
            soma::gameplay::DamageApplied{actor, firstTarget, 2},
            soma::gameplay::StatusGained{firstTarget, soma::gameplay::StatusId{"status.panic"}},
            soma::gameplay::ItemActivated{secondItem, actor},
            soma::gameplay::HealingApplied{actor, actor, 2}};

        const auto firstBatch = soma::presentation::BattleEventBatcher::takeNext(deferred);
        const auto secondBatch = soma::presentation::BattleEventBatcher::takeNext(deferred);

        checkTypes(firstBatch,
                   typesOf(soma::gameplay::ItemActivated{}, soma::gameplay::DamageApplied{}, soma::gameplay::StatusGained{}));
        checkTypes(secondBatch, typesOf(soma::gameplay::ItemActivated{}, soma::gameplay::HealingApplied{}));
        check(deferred.empty());
    }

    void representativeStreamPreservesPresentationOrder()
    {
        std::vector<GameplayEvent> deferred{
            soma::gameplay::TurnStarted{actor},
            soma::gameplay::AbilityUsed{actor, soma::gameplay::AbilityId{"ability.test"}},
            soma::gameplay::CriticalHit{firstTarget},
            soma::gameplay::DamageApplied{actor, firstTarget, 6},
            soma::gameplay::DamageApplied{actor, secondTarget, 4},
            soma::gameplay::StatusGained{firstTarget, soma::gameplay::StatusId{"status.panic"}},
            soma::gameplay::StatusGained{secondTarget, soma::gameplay::StatusId{"status.panic"}},
            soma::gameplay::StatusGained{firstTarget, soma::gameplay::StatusId{"status.marked"}},
            soma::gameplay::CombatantDefeated{firstTarget, actor}};

        const auto damageBatch = soma::presentation::BattleEventBatcher::takeNext(deferred);
        const auto panicBatch = soma::presentation::BattleEventBatcher::takeNext(deferred);
        const auto markedBatch = soma::presentation::BattleEventBatcher::takeNext(deferred);
        const auto defeatedBatch = soma::presentation::BattleEventBatcher::takeNext(deferred);

        checkTypes(damageBatch,
                   typesOf(soma::gameplay::CriticalHit{}, soma::gameplay::DamageApplied{}, soma::gameplay::DamageApplied{}));
        checkTypes(panicBatch, typesOf(soma::gameplay::StatusGained{}, soma::gameplay::StatusGained{}));
        check(std::get<soma::gameplay::StatusGained>(panicBatch.front()).status == soma::gameplay::StatusId{"status.panic"});
        checkTypes(markedBatch, typesOf(soma::gameplay::StatusGained{}));
        check(std::get<soma::gameplay::StatusGained>(markedBatch.front()).status == soma::gameplay::StatusId{"status.marked"});
        checkTypes(defeatedBatch, typesOf(soma::gameplay::CombatantDefeated{}));
        check(deferred.empty());
        check(soma::presentation::BattleEventBatcher::takeNext(deferred).empty());
    }
} // namespace

int main()
{
    metadataIsDiscardedBeforeProducingABatch();
    criticalHitsStayWithTheirDamageOrHealingOutcome();
    riposteTriggerIsPairedOnlyWithItsImmediateOutcome();
    itemActivationOwnsOutcomesUntilTheNextPresentationBoundary();
    representativeStreamPreservesPresentationOrder();
}
