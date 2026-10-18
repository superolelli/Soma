#pragma once

#include <cstdint>
#include <compare>
#include <string>
#include <vector>

namespace soma::presentation
{
    struct FormattedTextColor
    {
        std::uint8_t red{255};
        std::uint8_t green{255};
        std::uint8_t blue{255};

        auto operator<=>(const FormattedTextColor&) const = default;
    };

    enum class FormattedTextSymbol
    {
        none,
        arrowUp,
        arrowDown
    };

    struct FormattedTextSegment
    {
        std::string value;
        FormattedTextColor color;
        bool bold{};
        FormattedTextSymbol symbol{FormattedTextSymbol::none};
    };

    using FormattedTextLine = std::vector<FormattedTextSegment>;

    [[nodiscard]] std::vector<FormattedTextLine> parseFormattedText(
        const std::string& source);
}
