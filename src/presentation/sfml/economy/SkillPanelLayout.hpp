#pragma once

#include "content/economy/EconomyContent.hpp"

namespace soma::presentation::detail
{
    [[nodiscard]] inline content::Point addPoints(content::Point left, content::Point right)
    {
        return {left.x + right.x, left.y + right.y};
    }
} // namespace soma::presentation::detail
