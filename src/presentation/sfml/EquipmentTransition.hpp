#pragma once

#include "game/EquipmentBonusCalculator.hpp"

#include <SFML/Graphics.hpp>
#include <optional>

namespace soma::presentation
{
    struct ConnectionTransitionState
    {
        sf::Color color;
        float progress{};
        bool deactivating{};
    };

    /// Tracks which equipment connections changed and how far their fill animation has progressed.
    class EquipmentTransition
    {
      public:
        void start(const game::EquipmentBonusSummary& previous, const game::EquipmentBonusSummary& target);
        void reset();

        [[nodiscard]] sf::Color diamondColor(const game::EquipmentBonusSummary& current) const;
        [[nodiscard]] std::optional<ConnectionTransitionState> connectionState(
            std::size_t index,
            const game::EquipmentBonusSummary& current) const;

      private:
        [[nodiscard]] float elapsedSeconds() const;

        game::EquipmentBonusSummary previous_{};
        game::EquipmentBonusSummary target_{};
        mutable sf::Color displayedDiamond_{sf::Color::Black};
        sf::Color startingDiamond_{sf::Color::Black};
        mutable sf::Clock clock_;
        mutable bool active_{};
    };
} // namespace soma::presentation
