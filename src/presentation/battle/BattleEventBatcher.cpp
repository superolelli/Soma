#include "presentation/battle/BattleEventBatcher.hpp"

#include <algorithm>
#include <iterator>

namespace
{
    using Event = soma::gameplay::GameplayEvent;

    enum class OutcomeBatch
    {
        single,
        dodge,
        damage,
        healing,
        fatigue,
        statusGained,
        statusExpired,
        statusTriggered,
        resisted,
        buffsRemoved,
        debuffsRemoved,
        decayTransferred,
        decayDetonated,
        effectsProlonged,
        defeated
    };

    bool startsPresentationBatch(const Event& event)
    {
        return std::holds_alternative<soma::gameplay::TurnStarted>(event) ||
               std::holds_alternative<soma::gameplay::ItemActivated>(event) ||
               soma::presentation::BattleEventBatcher::isRiposteTrigger(event);
    }

    bool isPresentationMetadata(const Event& event)
    {
        return std::holds_alternative<soma::gameplay::TurnStarted>(event) ||
               std::holds_alternative<soma::gameplay::AbilityUsed>(event) ||
               std::holds_alternative<soma::gameplay::TargetRedirected>(event);
    }

    bool pairsWithFollowingResult(const Event& event)
    {
        return std::holds_alternative<soma::gameplay::CriticalHit>(event) ||
               std::holds_alternative<soma::gameplay::StatusTriggered>(event) ||
               soma::presentation::BattleEventBatcher::isRiposteTrigger(event);
    }

    bool isImmediateOutcome(const Event& event)
    {
        return std::holds_alternative<soma::gameplay::DamageApplied>(event) ||
               std::holds_alternative<soma::gameplay::HealingApplied>(event) ||
               std::holds_alternative<soma::gameplay::FatigueChanged>(event);
    }

    OutcomeBatch outcomeBatch(const std::vector<Event>& deferred)
    {
        const auto& event = deferred.front();
        if (std::holds_alternative<soma::gameplay::AttackDodged>(event))
        {
            return OutcomeBatch::dodge;
        }
        if (std::holds_alternative<soma::gameplay::DamageApplied>(event))
        {
            return OutcomeBatch::damage;
        }
        if (std::holds_alternative<soma::gameplay::HealingApplied>(event))
        {
            return OutcomeBatch::healing;
        }
        if (std::holds_alternative<soma::gameplay::FatigueChanged>(event))
        {
            return OutcomeBatch::fatigue;
        }
        if (std::holds_alternative<soma::gameplay::StatusGained>(event))
        {
            return OutcomeBatch::statusGained;
        }
        if (std::holds_alternative<soma::gameplay::StatusExpired>(event))
        {
            return OutcomeBatch::statusExpired;
        }
        if (std::holds_alternative<soma::gameplay::StatusTriggered>(event))
        {
            return OutcomeBatch::statusTriggered;
        }
        if (std::holds_alternative<soma::gameplay::EffectResisted>(event))
        {
            return OutcomeBatch::resisted;
        }
        if (std::holds_alternative<soma::gameplay::BuffsRemoved>(event))
        {
            return OutcomeBatch::buffsRemoved;
        }
        if (std::holds_alternative<soma::gameplay::DebuffsRemoved>(event))
        {
            return OutcomeBatch::debuffsRemoved;
        }
        if (std::holds_alternative<soma::gameplay::DecayTransferred>(event))
        {
            return OutcomeBatch::decayTransferred;
        }
        if (std::holds_alternative<soma::gameplay::DecayDetonated>(event))
        {
            return OutcomeBatch::decayDetonated;
        }
        if (std::holds_alternative<soma::gameplay::NegativeEffectsProlonged>(event))
        {
            return OutcomeBatch::effectsProlonged;
        }
        if (std::holds_alternative<soma::gameplay::CombatantDefeated>(event))
        {
            return OutcomeBatch::defeated;
        }
        if (!std::holds_alternative<soma::gameplay::CriticalHit>(event))
        {
            return OutcomeBatch::single;
        }

        const auto result = std::ranges::find_if(deferred, [](const auto& candidate) {
            return !std::holds_alternative<soma::gameplay::CriticalHit>(candidate);
        });
        if (result != deferred.end() && std::holds_alternative<soma::gameplay::DamageApplied>(*result))
        {
            return OutcomeBatch::damage;
        }
        if (result != deferred.end() && std::holds_alternative<soma::gameplay::HealingApplied>(*result))
        {
            return OutcomeBatch::healing;
        }
        return OutcomeBatch::single;
    }

    bool hasSameStatus(const Event& first, const Event& candidate, OutcomeBatch batch)
    {
        switch (batch)
        {
        case OutcomeBatch::statusGained:
            return std::get<soma::gameplay::StatusGained>(first).status ==
                   std::get<soma::gameplay::StatusGained>(candidate).status;
        case OutcomeBatch::statusExpired:
            return std::get<soma::gameplay::StatusExpired>(first).status ==
                   std::get<soma::gameplay::StatusExpired>(candidate).status;
        case OutcomeBatch::statusTriggered:
            return std::get<soma::gameplay::StatusTriggered>(first).status ==
                   std::get<soma::gameplay::StatusTriggered>(candidate).status;
        case OutcomeBatch::resisted:
            return std::get<soma::gameplay::EffectResisted>(first).status ==
                   std::get<soma::gameplay::EffectResisted>(candidate).status;
        default:
            return true;
        }
    }

    bool belongsToBatch(const Event& first, const Event& candidate, OutcomeBatch batch)
    {
        switch (batch)
        {
        case OutcomeBatch::damage:
            return std::holds_alternative<soma::gameplay::DamageApplied>(candidate) ||
                   std::holds_alternative<soma::gameplay::CriticalHit>(candidate);
        case OutcomeBatch::healing:
            return std::holds_alternative<soma::gameplay::HealingApplied>(candidate) ||
                   std::holds_alternative<soma::gameplay::CriticalHit>(candidate);
        case OutcomeBatch::dodge:
            return std::holds_alternative<soma::gameplay::AttackDodged>(candidate);
        case OutcomeBatch::fatigue:
            return std::holds_alternative<soma::gameplay::FatigueChanged>(candidate);
        case OutcomeBatch::statusGained:
            return std::holds_alternative<soma::gameplay::StatusGained>(candidate) && hasSameStatus(first, candidate, batch);
        case OutcomeBatch::statusExpired:
            return std::holds_alternative<soma::gameplay::StatusExpired>(candidate) && hasSameStatus(first, candidate, batch);
        case OutcomeBatch::statusTriggered:
            return std::holds_alternative<soma::gameplay::StatusTriggered>(candidate) && hasSameStatus(first, candidate, batch);
        case OutcomeBatch::resisted:
            return std::holds_alternative<soma::gameplay::EffectResisted>(candidate) && hasSameStatus(first, candidate, batch);
        case OutcomeBatch::buffsRemoved:
            return std::holds_alternative<soma::gameplay::BuffsRemoved>(candidate);
        case OutcomeBatch::debuffsRemoved:
            return std::holds_alternative<soma::gameplay::DebuffsRemoved>(candidate);
        case OutcomeBatch::decayTransferred:
            return std::holds_alternative<soma::gameplay::DecayTransferred>(candidate);
        case OutcomeBatch::decayDetonated:
            return std::holds_alternative<soma::gameplay::DecayDetonated>(candidate);
        case OutcomeBatch::effectsProlonged:
            return std::holds_alternative<soma::gameplay::NegativeEffectsProlonged>(candidate);
        case OutcomeBatch::defeated:
            return std::holds_alternative<soma::gameplay::CombatantDefeated>(candidate);
        case OutcomeBatch::single:
            return false;
        }
        return false;
    }
} // namespace

namespace soma::presentation
{
    bool BattleEventBatcher::isRiposteTrigger(const gameplay::GameplayEvent& event)
    {
        const auto* triggered = std::get_if<gameplay::StatusTriggered>(&event);
        return triggered != nullptr && triggered->status == gameplay::StatusId{"status.riposte"};
    }

    std::vector<gameplay::GameplayEvent> BattleEventBatcher::takeNext(std::vector<gameplay::GameplayEvent>& deferredEvents)
    {
        while (!deferredEvents.empty() && isPresentationMetadata(deferredEvents.front()))
        {
            deferredEvents.erase(deferredEvents.begin());
        }
        if (deferredEvents.empty())
        {
            return {};
        }

        auto nextBatch = std::next(deferredEvents.begin());
        if (std::holds_alternative<gameplay::ItemActivated>(deferredEvents.front()))
        {
            nextBatch = std::ranges::find_if(std::next(deferredEvents.begin()), deferredEvents.end(), startsPresentationBatch);
        }
        else if (isRiposteTrigger(deferredEvents.front()) && nextBatch != deferredEvents.end() && isImmediateOutcome(*nextBatch))
        {
            ++nextBatch;
        }
        else
        {
            const auto batch = outcomeBatch(deferredEvents);
            if (batch != OutcomeBatch::single)
            {
                nextBatch = std::ranges::find_if(std::next(deferredEvents.begin()), deferredEvents.end(), [&](const auto& candidate) {
                    return !belongsToBatch(deferredEvents.front(), candidate, batch);
                });
            }
            else if (pairsWithFollowingResult(deferredEvents.front()) && nextBatch != deferredEvents.end() &&
                     isImmediateOutcome(*nextBatch))
            {
                ++nextBatch;
            }
        }

        std::vector<gameplay::GameplayEvent> result;
        result.assign(std::make_move_iterator(deferredEvents.begin()), std::make_move_iterator(nextBatch));
        deferredEvents.erase(deferredEvents.begin(), nextBatch);
        return result;
    }
} // namespace soma::presentation
