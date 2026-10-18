#include "presentation/battle/BattleEventPresentation.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <unordered_map>

namespace
{
    using soma::gameplay::GameplayEvent;

    int eventPresentationRank(const GameplayEvent& event,
                              const std::set<soma::gameplay::EntityId>& damaged,
                              const std::set<soma::gameplay::EntityId>& healed,
                              const std::unordered_map<std::string, int>& statusOrder)
    {
        if (std::holds_alternative<soma::gameplay::AbilityUsed>(event) ||
            std::holds_alternative<soma::gameplay::TargetRedirected>(event))
        {
            return 0;
        }
        if (std::holds_alternative<soma::gameplay::AttackDodged>(event))
        {
            return 10;
        }
        if (std::holds_alternative<soma::gameplay::DamageApplied>(event))
        {
            return 20;
        }
        if (std::holds_alternative<soma::gameplay::HealingApplied>(event))
        {
            return 30;
        }
        if (const auto* critical = std::get_if<soma::gameplay::CriticalHit>(&event))
        {
            if (damaged.contains(critical->combatant))
            {
                return 20;
            }
            if (healed.contains(critical->combatant))
            {
                return 30;
            }
            return 40;
        }
        if (std::holds_alternative<soma::gameplay::FatigueChanged>(event))
        {
            return 45;
        }
        if (const auto* gained = std::get_if<soma::gameplay::StatusGained>(&event))
        {
            return 50 + statusOrder.at(gained->status.value());
        }
        if (const auto* resisted = std::get_if<soma::gameplay::EffectResisted>(&event))
        {
            return 100 + statusOrder.at(resisted->status.value());
        }
        if (std::holds_alternative<soma::gameplay::BuffsRemoved>(event))
        {
            return 160;
        }
        if (std::holds_alternative<soma::gameplay::DebuffsRemoved>(event))
        {
            return 161;
        }
        if (std::holds_alternative<soma::gameplay::DecayTransferred>(event))
        {
            return 170;
        }
        if (std::holds_alternative<soma::gameplay::DecayDetonated>(event))
        {
            return 171;
        }
        if (std::holds_alternative<soma::gameplay::NegativeEffectsProlonged>(event))
        {
            return 172;
        }
        if (std::holds_alternative<soma::gameplay::StatusTriggered>(event))
        {
            return 180;
        }
        if (std::holds_alternative<soma::gameplay::StatusExpired>(event))
        {
            return 190;
        }
        if (std::holds_alternative<soma::gameplay::CombatantDefeated>(event))
        {
            return 200;
        }
        if (std::holds_alternative<soma::gameplay::BattleFinished>(event))
        {
            return 250;
        }
        return 210;
    }
}

namespace soma::presentation
{
    std::vector<gameplay::EntityId> affectedCombatants(
        const std::vector<gameplay::GameplayEvent>& events)
    {
        std::set<gameplay::EntityId> ids;
        for (const auto& event : events)
        {
            if (std::holds_alternative<gameplay::TurnStarted>(event))
            {
                break;
            }
            if (const auto* damage = std::get_if<gameplay::DamageApplied>(&event))
                ids.insert(damage->target);
            else if (const auto* healing = std::get_if<gameplay::HealingApplied>(&event))
                ids.insert(healing->target);
            else if (const auto* redirected = std::get_if<gameplay::TargetRedirected>(&event))
                ids.insert(redirected->resolvedTarget);
            else if (const auto* status = std::get_if<gameplay::StatusGained>(&event))
                ids.insert(status->combatant);
            else if (const auto* dodge = std::get_if<gameplay::AttackDodged>(&event))
                ids.insert(dodge->target);
            else if (const auto* resisted = std::get_if<gameplay::EffectResisted>(&event))
                ids.insert(resisted->combatant);
        }
        return {ids.begin(), ids.end()};
    }

    bool isHostileImpact(const gameplay::EntityId& target,
                         const std::vector<gameplay::GameplayEvent>& events)
    {
        return std::ranges::any_of(events, [&target](const auto& event) {
            if (const auto* damage = std::get_if<gameplay::DamageApplied>(&event))
                return damage->target == target;
            if (const auto* dodge = std::get_if<gameplay::AttackDodged>(&event))
                return dodge->target == target;
            return false;
        });
    }

    std::string resistedStatusNotification(const gameplay::StatusId& status)
    {
        if (status == gameplay::StatusId{"status.confused"}) return "Verwirrung widerstanden!";
        if (status == gameplay::StatusId{"status.sleeping"}) return "Schlaf widerstanden!";
        if (status == gameplay::StatusId{"status.decay"}) return "Verfall widerstanden!";
        if (status == gameplay::StatusId{"status.hacked"}) return "Hacken widerstanden!";
        if (status == gameplay::StatusId{"status.debuffed"}) return "Debuff widerstanden!";
        return "Widerstanden!";
    }

    std::vector<gameplay::GameplayEvent> groupedForPresentation(std::vector<gameplay::GameplayEvent> events)
    {
        std::unordered_map<std::string, int> statusOrder;
        std::set<gameplay::EntityId> damaged;
        std::set<gameplay::EntityId> healed;
        auto nextStatusOrder = 0;
        for (const auto& event : events)
        {
            if (const auto* damage = std::get_if<gameplay::DamageApplied>(&event))
            {
                damaged.insert(damage->target);
            }
            if (const auto* healing = std::get_if<gameplay::HealingApplied>(&event))
            {
                healed.insert(healing->target);
            }
            const auto* gained = std::get_if<gameplay::StatusGained>(&event);
            const auto* resisted = std::get_if<gameplay::EffectResisted>(&event);
            const auto* status = gained != nullptr ? &gained->status : resisted != nullptr ? &resisted->status : nullptr;
            if (status != nullptr && !statusOrder.contains(status->value()))
            {
                statusOrder.emplace(status->value(), nextStatusOrder++);
            }
        }
        std::ranges::stable_sort(events, [&damaged, &healed, &statusOrder](const auto& left, const auto& right) {
            return eventPresentationRank(left, damaged, healed, statusOrder) < eventPresentationRank(right, damaged, healed, statusOrder);
        });
        return events;
    }
}
