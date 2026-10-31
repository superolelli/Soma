#include "simulation/battle/BattleSimulation.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace soma::simulation
{
    namespace
    {
        gameplay::StatusId statusId(std::string value)
        {
            return gameplay::StatusId{"status." + std::move(value)};
        }
    } // namespace

    void BattleSimulation::inflictRemainingDecay(CombatantState& actor, CombatantState& target, std::vector<gameplay::GameplayEvent>& events)
    {
        auto damage = 0;
        for (const auto& decay : target.decay)
        {
            damage += std::max(0, decay.damage) * std::max(0, decay.remainingTurns);
        }
        if (damage == 0)
        {
            return;
        }
        target.decay.clear();
        events.emplace_back(gameplay::DecayDetonated{target.id});
        events.emplace_back(gameplay::StatusTriggered{target.id, statusId("decay")});
        events.emplace_back(gameplay::StatusExpired{target.id, statusId("decay")});
        damage = std::max(0, standingModifiedValue(content::ModifierKind::damageReceived, target, damage));
        if (target.rageTurns > 0)
        {
            damage = static_cast<int>(std::round(damage * 1.5));
        }
        target.health = std::max(0, target.health - damage);
        updateNofaceBuff(target);
        recordDamageApplied(actor.id, target, damage, gameplay::DamageKind::decay, events);
        triggerTransfusions(target, events);
        expireTransfusionsWhenDecayEnds(target, events);
        emitDefeatIfNeeded(target, actor.id, events);
    }

    void BattleSimulation::transferDecayAfterDefeat(CombatantState& actor, CombatantState& defeated, std::vector<gameplay::GameplayEvent>& events)
    {
        if (defeated.decay.empty())
        {
            return;
        }
        std::vector<CombatantState*> candidates;
        for (auto& candidate : combatants_)
        {
            if (candidate.alive() && candidate.team != actor.team && candidate.id != defeated.id)
            {
                candidates.push_back(&candidate);
            }
        }
        if (candidates.empty())
        {
            return;
        }
        auto& recipient = *candidates.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(candidates.size()) - 1)));
        recipient.decay.insert(recipient.decay.end(), defeated.decay.begin(), defeated.decay.end());
        defeated.decay.clear();
        events.emplace_back(gameplay::DecayTransferred{defeated.id, recipient.id});
        events.emplace_back(gameplay::StatusExpired{defeated.id, statusId("decay")});
        expireTransfusionsWhenDecayEnds(defeated, events);
        recordStatusGained(recipient, statusId("decay"), events);
    }

    void BattleSimulation::triggerTransfusions(CombatantState& source, std::vector<gameplay::GameplayEvent>& events)
    {
        if (source.transfusions.empty())
        {
            return;
        }
        for (const auto& transfusion : source.transfusions)
        {
            std::vector<CombatantState*> candidates;
            for (auto& candidate : combatants_)
            {
                if (candidate.alive() && candidate.team != source.team)
                {
                    candidates.push_back(&candidate);
                }
            }
            if (candidates.empty())
            {
                continue;
            }
            auto& target = *candidates.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(candidates.size()) - 1)));
            events.emplace_back(gameplay::StatusTriggered{source.id, statusId("transfusion")});
            applyHealing(source.id, target, transfusion.healing, events);
        }
    }

    void BattleSimulation::expireTransfusionsWhenDecayEnds(CombatantState& target, std::vector<gameplay::GameplayEvent>& events)
    {
        if (target.transfusions.empty() || !target.decay.empty())
        {
            return;
        }
        target.transfusions.clear();
        events.emplace_back(gameplay::StatusExpired{target.id, statusId("transfusion")});
    }

    void BattleSimulation::removeDecayAndHealLivingPlayers(CombatantState& actor,
                                                           CombatantState& target,
                                                           std::vector<gameplay::GameplayEvent>& events)
    {
        auto outstandingDamage = 0;
        for (const auto& decay : target.decay)
        {
            outstandingDamage += std::max(0, decay.damage) * std::max(0, decay.remainingTurns);
        }
        if (outstandingDamage == 0)
        {
            return;
        }

        target.decay.clear();
        events.emplace_back(gameplay::StatusExpired{target.id, statusId("decay")});
        expireTransfusionsWhenDecayEnds(target, events);

        std::vector<CombatantState*> players;
        for (auto& combatant : combatants_)
        {
            if (combatant.alive() && combatant.team == gameplay::Team::players)
            {
                players.push_back(&combatant);
            }
        }
        if (players.empty())
        {
            return;
        }

        const auto healingPerPlayer = outstandingDamage / static_cast<int>(players.size());
        auto remainder = outstandingDamage % static_cast<int>(players.size());
        std::vector<CombatantState*> remainderCandidates = players;
        std::unordered_set<gameplay::EntityId> remainderRecipients;
        while (remainder-- > 0)
        {
            const auto index = static_cast<std::size_t>(random_.between(0, static_cast<int>(remainderCandidates.size()) - 1));
            remainderRecipients.insert(remainderCandidates.at(index)->id);
            remainderCandidates.erase(remainderCandidates.begin() + static_cast<std::ptrdiff_t>(index));
        }
        for (auto* player : players)
        {
            const auto remainderHealing = remainderRecipients.contains(player->id) ? 1 : 0;
            const auto healing = healingPerPlayer + remainderHealing;
            if (healing > 0)
            {
                applyHealing(actor.id, *player, healing, events);
            }
        }
    }

} // namespace soma::simulation
