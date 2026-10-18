#pragma once

#include "gameplay/events/GameplayEvent.hpp"

#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace soma::presentation
{
    struct PresentationStep
    {
        float durationSeconds{};
        std::string announcement;
        gameplay::EntityId subject;
        std::string animation;
        std::optional<int> healthChange;
        bool returnToIdle{};
    };

    /// Queues presentation steps so simultaneous simulation results remain understandable on screen.
    class BattlePresentationSequence
    {
      public:
        void enqueue(const std::vector<gameplay::GameplayEvent>& events);
        void update(float elapsedSeconds);

        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] bool stepChanged() const noexcept;
        [[nodiscard]] const PresentationStep* current() const noexcept;

      private:
        void addStep(PresentationStep step);

        std::deque<PresentationStep> steps_;
        float elapsedInCurrentStep_{};
        bool stepChanged_{};
    };
} // namespace soma::presentation
