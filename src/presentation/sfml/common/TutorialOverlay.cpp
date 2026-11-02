#include "presentation/sfml/common/TutorialOverlay.hpp"

#include "presentation/sfml/common/UiCursor.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <vector>

namespace soma::presentation
{
    namespace
    {
        constexpr sf::Vector2f panelSize{820.0F, 290.0F};
        constexpr sf::Vector2f buttonSize{220.0F, 52.0F};
        constexpr float screenMargin = 38.0F;
        constexpr float highlightClearance = 20.0F;

        struct OverlayLayout
        {
            sf::FloatRect panel;
            sf::FloatRect button;
            std::vector<sf::FloatRect> highlights;
        };

        sf::Text text(const sf::Font& font, const sf::String& value, unsigned int size, sf::Vector2f position)
        {
            sf::Text result{font, value, size};
            result.setPosition(position);
            result.setFillColor(sf::Color{39, 24, 15});
            return result;
        }

        void center(sf::Text& value, sf::Vector2f at)
        {
            const auto bounds = value.getLocalBounds();
            value.setOrigin(bounds.position + bounds.size / 2.0F);
            value.setPosition(at);
        }

        sf::FloatRect expanded(sf::FloatRect bounds, float amount)
        {
            return {bounds.position - sf::Vector2f{amount, amount}, bounds.size + sf::Vector2f{amount * 2.0F, amount * 2.0F}};
        }

        float overlapArea(const sf::FloatRect& left, const sf::FloatRect& right)
        {
            const auto width = std::max(0.0F,
                                        std::min(left.position.x + left.size.x, right.position.x + right.size.x) -
                                            std::max(left.position.x, right.position.x));
            const auto height = std::max(0.0F,
                                         std::min(left.position.y + left.size.y, right.position.y + right.size.y) -
                                             std::max(left.position.y, right.position.y));
            return width * height;
        }

        sf::Vector2f clampedPanelPosition(sf::Vector2f requested, sf::Vector2f origin, sf::Vector2f viewSize)
        {
            return {std::clamp(requested.x, origin.x + screenMargin, origin.x + viewSize.x - panelSize.x - screenMargin),
                    std::clamp(requested.y, origin.y + screenMargin, origin.y + viewSize.y - panelSize.y - screenMargin)};
        }

        OverlayLayout layoutFor(const content::TutorialMessage& message, const sf::View& view)
        {
            const auto origin = view.getCenter() - view.getSize() / 2.0F;
            std::vector<sf::FloatRect> highlights;
            highlights.reserve(message.highlights.size());
            for (const auto& source : message.highlights)
            {
                highlights.push_back({origin + sf::Vector2f{static_cast<float>(source.x), static_cast<float>(source.y)},
                                      {static_cast<float>(source.width), static_cast<float>(source.height)}});
            }

            const auto centeredX = origin.x + (view.getSize().x - panelSize.x) / 2.0F;
            const auto centeredY = origin.y + (view.getSize().y - panelSize.y) / 2.0F;
            std::vector<sf::Vector2f> candidates{
                {centeredX, origin.y + view.getSize().y - panelSize.y - screenMargin},
                {centeredX, origin.y + screenMargin},
                {origin.x + view.getSize().x - panelSize.x - screenMargin, centeredY},
                {origin.x + screenMargin, centeredY},
                {origin.x + screenMargin, origin.y + screenMargin},
                {origin.x + view.getSize().x - panelSize.x - screenMargin, origin.y + screenMargin},
                {origin.x + screenMargin, origin.y + view.getSize().y - panelSize.y - screenMargin},
                {origin.x + view.getSize().x - panelSize.x - screenMargin,
                 origin.y + view.getSize().y - panelSize.y - screenMargin}};

            for (const auto& highlight : highlights)
            {
                candidates.push_back(clampedPanelPosition({highlight.getCenter().x - panelSize.x / 2.0F,
                                                           highlight.position.y - panelSize.y - highlightClearance},
                                                          origin,
                                                          view.getSize()));
                candidates.push_back(clampedPanelPosition({highlight.getCenter().x - panelSize.x / 2.0F,
                                                           highlight.position.y + highlight.size.y + highlightClearance},
                                                          origin,
                                                          view.getSize()));
                candidates.push_back(clampedPanelPosition({highlight.position.x - panelSize.x - highlightClearance,
                                                           highlight.getCenter().y - panelSize.y / 2.0F},
                                                          origin,
                                                          view.getSize()));
                candidates.push_back(clampedPanelPosition({highlight.position.x + highlight.size.x + highlightClearance,
                                                           highlight.getCenter().y - panelSize.y / 2.0F},
                                                          origin,
                                                          view.getSize()));
            }

            auto bestScore = std::numeric_limits<float>::max();
            auto bestPosition = candidates.front();
            for (std::size_t index = 0; index < candidates.size(); ++index)
            {
                const sf::FloatRect candidate{clampedPanelPosition(candidates.at(index), origin, view.getSize()), panelSize};
                auto score = static_cast<float>(index);
                for (const auto& highlight : highlights)
                {
                    score += overlapArea(candidate, expanded(highlight, highlightClearance)) * 1000.0F;
                }
                if (score < bestScore)
                {
                    bestScore = score;
                    bestPosition = candidate.position;
                }
            }

            const sf::FloatRect panel{bestPosition, panelSize};
            const sf::FloatRect button{{panel.position.x + panel.size.x - buttonSize.x - 38.0F,
                                        panel.position.y + panel.size.y - buttonSize.y - 24.0F},
                                       buttonSize};
            return {panel, button, std::move(highlights)};
        }

        sf::Vector2f closestPoint(const sf::FloatRect& bounds, sf::Vector2f point)
        {
            return {std::clamp(point.x, bounds.position.x, bounds.position.x + bounds.size.x),
                    std::clamp(point.y, bounds.position.y, bounds.position.y + bounds.size.y)};
        }

        void drawConnector(sf::RenderWindow& window, const sf::FloatRect& panel, const sf::FloatRect& highlight)
        {
            const auto start = closestPoint(panel, highlight.getCenter());
            const auto end = closestPoint(highlight, panel.getCenter());
            const auto delta = end - start;
            const auto length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            if (length <= 1.0F) return;

            const auto rotation = sf::radians(std::atan2(delta.y, delta.x));
            sf::RectangleShape shadow{{length, 8.0F}};
            shadow.setOrigin({0.0F, 4.0F});
            shadow.setPosition(start + sf::Vector2f{2.0F, 2.0F});
            shadow.setRotation(rotation);
            shadow.setFillColor(sf::Color{39, 24, 15, 180});
            window.draw(shadow);

            sf::RectangleShape connector{{length, 4.0F}};
            connector.setOrigin({0.0F, 2.0F});
            connector.setPosition(start);
            connector.setRotation(rotation);
            connector.setFillColor(sf::Color{255, 191, 45});
            window.draw(connector);
        }
    } // namespace

    TutorialOverlay::TutorialOverlay(sf::RenderWindow& window, const sf::Font& font)
        : window_(window), font_(font)
    {
    }

    bool TutorialOverlay::show(const content::TutorialMessage& message, const std::function<void()>& drawBackground) const
    {
        while (window_.isOpen())
        {
            const auto currentLayout = layoutFor(message, window_.getView());
            while (const auto event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window_.close();
                    return false;
                }
                if (const auto* key = event->getIf<sf::Event::KeyPressed>();
                    key && (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space))
                {
                    return true;
                }
                if (const auto* click = event->getIf<sf::Event::MouseButtonReleased>();
                    click && click->button == sf::Mouse::Button::Left &&
                    currentLayout.button.contains(window_.mapPixelToCoords(click->position)))
                {
                    return true;
                }
            }
            const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
            setUiCursor(window_, currentLayout.button.contains(mouse));
            draw(message, drawBackground, mouse);
        }
        return false;
    }

    sf::String TutorialOverlay::wrapped(std::string_view source, unsigned int size, float maximumWidth) const
    {
        std::istringstream words{std::string{source}};
        std::string word;
        std::string line;
        std::string result;
        while (words >> word)
        {
            const auto candidate = line.empty() ? word : line + " " + word;
            auto measurement = text(font_, sf::String::fromUtf8(candidate.begin(), candidate.end()), size, {});
            if (!line.empty() && measurement.getLocalBounds().size.x > maximumWidth)
            {
                result += line + '\n';
                line = word;
            }
            else
            {
                line = candidate;
            }
        }
        result += line;
        return sf::String::fromUtf8(result.begin(), result.end());
    }

    void TutorialOverlay::draw(const content::TutorialMessage& message,
                               const std::function<void()>& drawBackground,
                               sf::Vector2f mouse) const
    {
        drawBackground();
        const auto layout = layoutFor(message, window_.getView());

        for (const auto& highlight : layout.highlights)
        {
            drawConnector(window_, layout.panel, highlight);
            sf::RectangleShape marker{highlight.size};
            marker.setPosition(highlight.position);
            marker.setFillColor(sf::Color{255, 191, 45, 22});
            marker.setOutlineColor(sf::Color{255, 191, 45});
            marker.setOutlineThickness(5.0F);
            window_.draw(marker);
        }

        sf::RectangleShape shadow{layout.panel.size};
        shadow.setPosition(layout.panel.position + sf::Vector2f{8.0F, 8.0F});
        shadow.setFillColor(sf::Color{0, 0, 0, 130});
        window_.draw(shadow);
        sf::RectangleShape panel{layout.panel.size};
        panel.setPosition(layout.panel.position);
        panel.setFillColor(sf::Color{224, 190, 128});
        panel.setOutlineColor(sf::Color{49, 25, 10});
        panel.setOutlineThickness(7.0F);
        window_.draw(panel);

        auto title = text(font_,
                          sf::String::fromUtf8(message.title.begin(), message.title.end()),
                          34U,
                          layout.panel.position + sf::Vector2f{40.0F, 24.0F});
        title.setStyle(sf::Text::Style::Bold);
        window_.draw(title);
        window_.draw(text(font_,
                          wrapped(message.text, 23U, layout.panel.size.x - 80.0F),
                          23U,
                          layout.panel.position + sf::Vector2f{40.0F, 82.0F}));

        sf::RectangleShape button{layout.button.size};
        button.setPosition(layout.button.position);
        button.setFillColor(layout.button.contains(mouse) ? sf::Color{139, 75, 29} : sf::Color{104, 52, 20});
        button.setOutlineColor(sf::Color{43, 21, 8});
        button.setOutlineThickness(4.0F);
        window_.draw(button);
        auto buttonText = text(font_, sf::String::fromUtf8(message.button.begin(), message.button.end()), 23U, {});
        buttonText.setFillColor(sf::Color::White);
        center(buttonText, layout.button.getCenter());
        window_.draw(buttonText);
        window_.display();
    }
} // namespace soma::presentation
