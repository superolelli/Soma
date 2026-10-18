#pragma once

#include "gameplay/events/GameplayEvent.hpp"

#include <string>
#include <vector>

namespace soma::presentation
{
    [[nodiscard]] std::vector<gameplay::EntityId> affectedCombatants(
        const std::vector<gameplay::GameplayEvent>& events);
    [[nodiscard]] bool isHostileImpact(
        const gameplay::EntityId& target,
        const std::vector<gameplay::GameplayEvent>& events);
    [[nodiscard]] std::string resistedStatusNotification(const gameplay::StatusId& status);
    [[nodiscard]] std::vector<gameplay::GameplayEvent> groupedForPresentation(
        std::vector<gameplay::GameplayEvent> events);
}
