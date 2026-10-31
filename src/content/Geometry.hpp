#pragma once

#include <compare>

namespace soma::content
{
    /// Integer position in authored presentation coordinates.
    struct Point
    {
        int x{};
        int y{};

        auto operator<=>(const Point&) const = default;
    };

    /// Integer rectangle in authored presentation coordinates.
    struct Rectangle
    {
        int x{};
        int y{};
        int width{};
        int height{};
    };
} // namespace soma::content
