#include "presentation/sfml/common/FormattedTextLayout.hpp"

#include <algorithm>
#include <sstream>

namespace soma::presentation
{
    namespace
    {
        sf::Text textFor(const sf::Font& font, const FormattedTextSegment& segment, unsigned int characterSize)
        {
            sf::Text text{font, sf::String::fromUtf8(segment.value.begin(), segment.value.end()), characterSize};
            if (segment.bold)
            {
                text.setStyle(sf::Text::Bold);
            }
            return text;
        }

        float widthOf(const sf::Font& font, const FormattedTextSegment& segment, unsigned int characterSize)
        {
            if (segment.symbol != FormattedTextSymbol::none)
            {
                const auto leadingGap = segment.value.empty() ? 0.0F : static_cast<float>(characterSize) * 0.3F;
                return leadingGap + static_cast<float>(characterSize) * 0.55F;
            }
            return textFor(font, segment, characterSize).getLocalBounds().size.x;
        }

        void appendSegment(FormattedTextLine& line, FormattedTextSegment segment)
        {
            if (!line.empty() && line.back().symbol == FormattedTextSymbol::none && segment.symbol == FormattedTextSymbol::none &&
                line.back().color == segment.color && line.back().bold == segment.bold)
            {
                line.back().value += segment.value;
                return;
            }
            line.push_back(std::move(segment));
        }

        void drawArrow(sf::RenderWindow& window,
                       const FormattedTextSegment& segment,
                       unsigned int characterSize,
                       float lineHeight,
                       sf::Vector2f position)
        {
            const auto width = static_cast<float>(characterSize) * 0.55F;
            const auto height = static_cast<float>(characterSize) * 0.8F;
            const auto top = (lineHeight - height) / 2.0F;
            const auto down = segment.symbol == FormattedTextSymbol::arrowDown;
            const auto y = [down, height](float upValue) {
                return down ? height - upValue : upValue;
            };

            sf::ConvexShape arrow{7};
            arrow.setPoint(0, {width * 0.5F, y(0.0F)});
            arrow.setPoint(1, {width, y(height * 0.42F)});
            arrow.setPoint(2, {width * 0.68F, y(height * 0.42F)});
            arrow.setPoint(3, {width * 0.68F, y(height)});
            arrow.setPoint(4, {width * 0.32F, y(height)});
            arrow.setPoint(5, {width * 0.32F, y(height * 0.42F)});
            arrow.setPoint(6, {0.0F, y(height * 0.42F)});
            arrow.setPosition(position + sf::Vector2f{0.0F, top});
            arrow.setFillColor({segment.color.red, segment.color.green, segment.color.blue});
            window.draw(arrow);
        }
    }

    FormattedTextLayout layoutFormattedText(const sf::Font& font,
                                            const std::string& value,
                                            unsigned int characterSize,
                                            float maximumWidth,
                                            float lineHeight)
    {
        FormattedTextLayout result;
        result.characterSize = characterSize;
        result.lineHeight = lineHeight;

        auto maximumLineWidth = 0.0F;
        for (const auto& sourceLine : parseFormattedText(value))
        {
            result.lines.emplace_back();
            auto currentWidth = 0.0F;
            for (const auto& sourceSegment : sourceLine)
            {
                if (sourceSegment.symbol != FormattedTextSymbol::none)
                {
                    auto segment = sourceSegment;
                    segment.value = result.lines.back().empty() ? "" : " ";
                    auto segmentWidth = widthOf(font, segment, characterSize);
                    if (!result.lines.back().empty() && currentWidth + segmentWidth > maximumWidth)
                    {
                        maximumLineWidth = std::max(maximumLineWidth, currentWidth);
                        result.lines.emplace_back();
                        currentWidth = 0.0F;
                        segment.value.clear();
                        segmentWidth = widthOf(font, segment, characterSize);
                    }
                    result.lines.back().push_back(std::move(segment));
                    currentWidth += segmentWidth;
                    continue;
                }
                std::istringstream words{sourceSegment.value};
                std::string word;
                while (words >> word)
                {
                    FormattedTextSegment segment{(result.lines.back().empty() ? "" : " ") + word,
                                                 sourceSegment.color,
                                                 sourceSegment.bold};
                    auto segmentWidth = widthOf(font, segment, characterSize);
                    if (!result.lines.back().empty() && currentWidth + segmentWidth > maximumWidth)
                    {
                        maximumLineWidth = std::max(maximumLineWidth, currentWidth);
                        result.lines.emplace_back();
                        currentWidth = 0.0F;
                        segment.value = std::move(word);
                        segmentWidth = widthOf(font, segment, characterSize);
                    }
                    appendSegment(result.lines.back(), std::move(segment));
                    currentWidth += segmentWidth;
                }
            }
            maximumLineWidth = std::max(maximumLineWidth, currentWidth);
        }
        result.size = {maximumLineWidth, lineHeight * static_cast<float>(result.lines.size())};
        return result;
    }

    void drawFormattedText(sf::RenderWindow& window,
                           const sf::Font& font,
                           const FormattedTextLayout& layout,
                           sf::Vector2f position)
    {
        auto y = position.y;
        for (const auto& line : layout.lines)
        {
            auto x = position.x;
            for (const auto& segment : line)
            {
                if (segment.symbol != FormattedTextSymbol::none)
                {
                    const auto leadingGap = segment.value.empty() ? 0.0F : static_cast<float>(layout.characterSize) * 0.3F;
                    x += leadingGap;
                    drawArrow(window, segment, layout.characterSize, layout.lineHeight, {x, y});
                    x += static_cast<float>(layout.characterSize) * 0.55F;
                    continue;
                }
                auto text = textFor(font, segment, layout.characterSize);
                text.setPosition({x, y});
                text.setFillColor({segment.color.red, segment.color.green, segment.color.blue});
                window.draw(text);
                x += text.getLocalBounds().size.x;
            }
            y += layout.lineHeight;
        }
    }
}
