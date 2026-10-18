#include "presentation/battle/BattlePresentationSequence.hpp"

#include <type_traits>

namespace soma::presentation
{
    void BattlePresentationSequence::enqueue(
        const std::vector<gameplay::GameplayEvent>& events)
    {
        const auto wasEmpty = steps_.empty();

        for (const auto& event : events)
        {
            std::visit(
                [this](const auto& value) {
                    using Event = std::decay_t<decltype(value)>;

                    if constexpr (std::is_same_v<Event, gameplay::AbilityUsed>)
                    {
                        std::string animation = "all_on_one";
                        std::string announcement = "Simon greift an";

                        if (value.ability == gameplay::AbilityId{"ability.simon.first_aid"})
                        {
                            animation = "repair";
                            announcement = "Simon: Erste Hilfe";
                        }
                        else if (value.ability ==
                                 gameplay::AbilityId{"ability.enemy.basic_attack"})
                        {
                            animation = "bang";
                            announcement = "Gesetzloser: Bang!";
                        }

                        addStep({1.35F,
                                 std::move(announcement),
                                 value.combatant,
                                 std::move(animation),
                                 std::nullopt,
                                 true});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::AttackDodged>)
                    {
                        addStep({1.0F,
                                 "Ausgewichen!",
                                 value.target,
                                 "attacked",
                                 std::nullopt,
                                 true});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::DamageApplied>)
                    {
                        addStep({1.1F,
                                 "-" + std::to_string(value.amount) + " Leben",
                                 value.target,
                                 "attacked",
                                 -value.amount,
                                 true});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::HealingApplied>)
                    {
                        addStep({1.1F,
                                 "+" + std::to_string(value.amount) + " Leben",
                                 value.target,
                                 "attacked_friendly",
                                 value.amount,
                                 true});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::StatusGained>)
                    {
                        addStep({0.9F,
                                 "Status: " + value.status.value(),
                                 value.combatant,
                                 {},
                                 std::nullopt,
                                 false});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::CombatantDefeated>)
                    {
                        addStep({2.0F,
                                 "Besiegt",
                                 value.combatant,
                                 "dying",
                                 std::nullopt,
                                 false});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::TurnStarted>)
                    {
                        addStep({0.55F,
                                 "Nächster Zug",
                                 value.combatant,
                                 {},
                                 std::nullopt,
                                 false});
                    }
                    else if constexpr (std::is_same_v<Event, gameplay::BattleFinished>)
                    {
                        const auto result = value.winningTeam == gameplay::Team::players
                                                ? "Sieg!"
                                                : "Niederlage";
                        addStep({3.0F,
                                 result,
                                 gameplay::EntityId{"battle"},
                                 {},
                                 std::nullopt,
                                 false});
                    }
                },
                event);
        }

        if (wasEmpty && !steps_.empty())
        {
            elapsedInCurrentStep_ = 0.0F;
            stepChanged_ = true;
        }
    }

    void BattlePresentationSequence::update(float elapsedSeconds)
    {
        stepChanged_ = false;
        if (steps_.empty())
        {
            return;
        }

        elapsedInCurrentStep_ += elapsedSeconds;
        if (elapsedInCurrentStep_ < steps_.front().durationSeconds)
        {
            return;
        }

        elapsedInCurrentStep_ = 0.0F;
        steps_.pop_front();
        stepChanged_ = true;
    }

    bool BattlePresentationSequence::empty() const noexcept
    {
        return steps_.empty();
    }

    bool BattlePresentationSequence::stepChanged() const noexcept
    {
        return stepChanged_;
    }

    const PresentationStep* BattlePresentationSequence::current() const noexcept
    {
        return steps_.empty() ? nullptr : &steps_.front();
    }

    void BattlePresentationSequence::addStep(PresentationStep step)
    {
        steps_.push_back(std::move(step));
    }
} // namespace soma::presentation
