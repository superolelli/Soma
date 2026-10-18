#include "presentation/text/FormattedText.hpp"

#include <algorithm>
#include <cctype>

namespace soma::presentation
{
    std::vector<FormattedTextLine> parseFormattedText(const std::string& source)
    {
        std::vector<FormattedTextLine> lines(1);
        auto color = FormattedTextColor{};
        auto bold = false;
        std::string value;
        const auto flush = [&] {
            if (!value.empty())
            {
                lines.back().push_back({std::move(value), color, bold});
                value.clear();
            }
        };

        for (std::size_t index = 0; index < source.size();)
        {
            if (source.at(index) == '\n')
            {
                flush();
                lines.emplace_back();
                ++index;
                continue;
            }
            if (source.at(index) == '*')
            {
                flush();
                bold = !bold;
                ++index;
                continue;
            }
            if (source.compare(index, 10, "[arrow-up]") == 0)
            {
                flush();
                lines.back().push_back({{}, color, bold, FormattedTextSymbol::arrowUp});
                index += 10;
                continue;
            }
            if (source.compare(index, 12, "[arrow-down]") == 0)
            {
                flush();
                lines.back().push_back({{}, color, bold, FormattedTextSymbol::arrowDown});
                index += 12;
                continue;
            }
            if (source.compare(index, 7, "#white ") == 0)
            {
                flush();
                color = {};
                index += 7;
                continue;
            }
            if (source.at(index) == '#' && index + 7 <= source.size())
            {
                const auto hex = source.substr(index + 1, 6);
                const auto isHex = std::ranges::all_of(hex, [](unsigned char character) {
                    return std::isxdigit(character) != 0;
                });
                if (isHex)
                {
                    flush();
                    const auto numeric = static_cast<unsigned long>(
                        std::stoul(hex, nullptr, 16));
                    color = {
                        static_cast<std::uint8_t>((numeric >> 16U) & 0xFFU),
                        static_cast<std::uint8_t>((numeric >> 8U) & 0xFFU),
                        static_cast<std::uint8_t>(numeric & 0xFFU)};
                    index += 7;
                    if (index < source.size() && source.at(index) == ' ')
                    {
                        ++index;
                    }
                    continue;
                }
            }
            value.push_back(source.at(index));
            ++index;
        }
        flush();
        return lines;
    }
}
