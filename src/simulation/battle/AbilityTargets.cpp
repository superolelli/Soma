#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>

namespace soma::simulation
{
    namespace
    {
        constexpr int slotsPerTeam = 4;

        int firstSlotInGroup(int slot)
        {
            return slot / slotsPerTeam * slotsPerTeam;
        }
    } // namespace

    bool isValidPrimaryTarget(const content::Targeting& targeting,
                              const CombatantState& actor,
                              const CombatantState& candidate)
    {
        if (!candidate.alive()) return false;
        if (targeting.requiresRevengeMarker && !candidate.fistOfRevengeTarget) return false;
        if (targeting.excludesActor && candidate.id == actor.id) return false;

        switch (targeting.relation)
        {
        case content::TargetRelation::self:
            return candidate.id == actor.id;
        case content::TargetRelation::ownTeam:
            return candidate.team == actor.team;
        case content::TargetRelation::opposingTeam:
            return candidate.team != actor.team;
        case content::TargetRelation::eitherTeam:
            return true;
        case content::TargetRelation::everyoneElse:
            return candidate.id != actor.id;
        }
        return false;
    }

    std::vector<gameplay::EntityId> abilityTargets(const content::Targeting& targeting,
                                                    const CombatantState& actor,
                                                    const CombatantState& primary,
                                                    const std::vector<CombatantState>& combatants)
    {
        if (targeting.relation == content::TargetRelation::self)
        {
            return {actor.id};
        }

        std::vector<gameplay::EntityId> result;
        if (targeting.relation == content::TargetRelation::everyoneElse)
        {
            for (const auto& candidate : combatants)
            {
                if (candidate.alive() && candidate.id != actor.id) result.push_back(candidate.id);
            }
            return result;
        }

        if (targeting.scope == content::TargetScope::team)
        {
            for (const auto& candidate : combatants)
            {
                if (candidate.alive() && candidate.team == primary.team && (!targeting.excludesActor || candidate.id != actor.id))
                {
                    result.push_back(candidate.id);
                }
            }
            return result;
        }

        if (targeting.scope == content::TargetScope::randomSingle)
        {
            for (const auto& candidate : combatants)
            {
                if (candidate.alive() && candidate.team == primary.team) result.push_back(candidate.id);
            }
            return result;
        }

        result.push_back(primary.id);
        if (targeting.scope != content::TargetScope::count) return result;

        const auto firstSlot = firstSlotInGroup(primary.slot);
        const auto primaryOffset = primary.slot - firstSlot;
        const auto selectedSlotCount = std::min(targeting.targetCount, slotsPerTeam);
        for (auto offset = 1; offset < selectedSlotCount; ++offset)
        {
            const auto slot = firstSlot + (primaryOffset + offset) % slotsPerTeam;
            const auto candidate = std::ranges::find_if(combatants, [&primary, slot](const auto& combatant) {
                return combatant.alive() && combatant.team == primary.team && combatant.slot == slot;
            });
            if (candidate != combatants.end())
            {
                result.push_back(candidate->id);
            }
        }
        return result;
    }

    bool targetsActor(const content::Targeting& targeting,
                      const CombatantState& actor,
                      const CombatantState& primary)
    {
        if (targeting.relation == content::TargetRelation::self ||
            targeting.relation == content::TargetRelation::everyoneElse)
        {
            return targeting.relation == content::TargetRelation::self;
        }
        if (targeting.scope == content::TargetScope::team)
        {
            return actor.team == primary.team;
        }
        return actor.id == primary.id;
    }
} // namespace soma::simulation
