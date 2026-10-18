#include "presentation/battle/BattlePresentation.hpp"

#include "presentation/battle/BattleEventPresentation.hpp"

#include <type_traits>

namespace soma::presentation
{
    std::vector<PresentationCue> BattlePresentation::map(
        const std::vector<gameplay::GameplayEvent>& events) const
    {
        std::vector<PresentationCue> cues;

        for (const auto& event : events)
        {
            std::visit(
                [&cues](const auto& value) {
                    using Event = std::decay_t<decltype(value)>;

                    if constexpr (std::is_same_v<Event, gameplay::AbilityUsed>)
                    {
                        cues.push_back({CueKind::animation, value.combatant, "ability"});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::AttackDodged>)
                    {
                        cues.push_back({CueKind::animation, value.target, "attacked"});
                        cues.push_back({CueKind::notification, value.target, "Ausgewichen!"});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::DamageApplied>)
                    {
                        cues.push_back({CueKind::animation, value.target, "attacked"});
                        cues.push_back({CueKind::notification,
                                        value.target,
                                        "-" + std::to_string(value.amount)});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::HealingApplied>)
                    {
                        cues.push_back({CueKind::animation, value.target, "attacked_friendly"});
                        cues.push_back({CueKind::notification,
                                        value.target,
                                        "+" + std::to_string(value.amount)});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::StatusGained>)
                    {
                        cues.push_back({CueKind::notification,
                                        value.combatant,
                                        value.status.value()});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::EffectResisted>)
                    {
                        cues.push_back({CueKind::notification, value.combatant, resistedStatusNotification(value.status)});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::CombatantDefeated>)
                    {
                        cues.push_back({CueKind::animation, value.combatant, "dying"});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::RuleActivated>)
                    {
                        cues.push_back({CueKind::notification, value.trigger, "Regel aktiviert!"});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::ItemActivated>)
                    {
                        cues.push_back({CueKind::notification, value.carrier, "Gegenstand aktiviert!"});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::BattleFinished>)
                    {
                        const auto message = value.winningTeam == gameplay::Team::players
                                                 ? "Sieg!"
                                                 : "Niederlage";
                        cues.push_back(
                            {CueKind::notification, gameplay::EntityId{"battle"}, message});
                    }
                },
                event);
        }

        return cues;
    }
} // namespace soma::presentation
