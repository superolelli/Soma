#include "presentation/sfml/EquipmentTransition.hpp"

#include <algorithm>

namespace soma::presentation
{
    namespace
    {
        sf::Color itemColor(gameplay::ItemColor color, std::uint8_t alpha = 255)
        {
            return {color.red, color.green, color.blue, alpha};
        }

        std::uint8_t blend(std::uint8_t from, std::uint8_t to, float progress)
        {
            return static_cast<std::uint8_t>(static_cast<float>(from) + (static_cast<float>(to) - from) * progress);
        }
    } // namespace

    void EquipmentTransition::start(const game::EquipmentBonusSummary& previous, const game::EquipmentBonusSummary& target)
    {
        previous_ = previous;
        target_ = target;
        startingDiamond_ = displayedDiamond_;
        clock_.restart();
        active_ = true;
    }

    void EquipmentTransition::reset()
    {
        active_ = false;
    }

    sf::Color EquipmentTransition::diamondColor(const game::EquipmentBonusSummary& current) const
    {
        if (!active_)
        {
            displayedDiamond_ = itemColor(current.diamondColor);
            return displayedDiamond_;
        }
        const auto progress = std::clamp(elapsedSeconds() / 3.0F, 0.0F, 1.0F);
        const auto target = itemColor(target_.diamondColor);
        displayedDiamond_ = {blend(startingDiamond_.r, target.r, progress),
                             blend(startingDiamond_.g, target.g, progress),
                             blend(startingDiamond_.b, target.b, progress)};
        active_ = progress < 1.0F;
        return displayedDiamond_;
    }

    std::optional<ConnectionTransitionState> EquipmentTransition::connectionState(
        std::size_t index,
        const game::EquipmentBonusSummary& current) const
    {
        const auto& target = active_ ? target_.connections.at(index) : current.connections.at(index);
        const auto& previous = active_ ? previous_.connections.at(index) : target;
        if (!target.active && !previous.active)
        {
            return std::nullopt;
        }
        const auto changed = target.active != previous.active || (target.active && target.color != previous.color);
        const auto elapsed = elapsedSeconds();
        const auto color = target.active ? target.color : previous.color;
        const auto deactivating = previous.active && !target.active;
        const auto progress = active_ && changed ? std::clamp(elapsed / (deactivating ? 1.0F : 3.0F), 0.0F, 1.0F) : 1.0F;
        return ConnectionTransitionState{itemColor(color), progress, deactivating};
    }

    float EquipmentTransition::elapsedSeconds() const
    {
        return clock_.getElapsedTime().asSeconds();
    }
} // namespace soma::presentation
