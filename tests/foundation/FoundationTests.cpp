#include "gameplay/random/SeededRandom.hpp"
#include "simulation/rules/ModifierPipeline.hpp"
#include "simulation/rules/RuleEventQueue.hpp"

#include <cstdlib>

namespace
{
    void check(bool condition)
    {
        if (!condition)
        {
            std::abort();
        }
    }
} // namespace

int main()
{
    soma::gameplay::SeededRandom first{12345};
    soma::gameplay::SeededRandom second{12345};
    for (int index = 0; index < 32; ++index)
    {
        check(first.between(-5, 9) == second.between(-5, 9));
    }

    soma::simulation::ModifierPipeline modifiers;
    modifiers.add({soma::gameplay::RuleId{"rule.multiply"},
                   soma::simulation::ModifierOperation::multiplicative,
                   2.0,
                   0});
    modifiers.add({soma::gameplay::RuleId{"rule.add"},
                   soma::simulation::ModifierOperation::additive,
                   3.0,
                   0});
    modifiers.add({soma::gameplay::RuleId{"rule.cap"},
                   soma::simulation::ModifierOperation::clampMaximum,
                   20.0,
                   0});
    const auto modified = modifiers.evaluate(10.0);
    check(modified.value == 20.0);
    check(modified.trace.size() == 3);
    check(modified.trace.front().source == soma::gameplay::RuleId{"rule.add"});

    soma::simulation::RuleEventQueue queue{2, 8};
    check(queue.enqueue(soma::gameplay::TurnStarted{soma::gameplay::EntityId{"combatant.test"}}));
    const auto completed = queue.drain([](const auto& event, auto& targetQueue) {
        static_cast<void>(targetQueue.enqueue(event.event, event.depth + 1));
    });
    check(!completed);
    check(queue.limitReached());

}
