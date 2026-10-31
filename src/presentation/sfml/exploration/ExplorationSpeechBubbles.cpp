#include "presentation/sfml/exploration/ExplorationSpeechBubbles.hpp"

#include <algorithm>
#include <sstream>

namespace soma::presentation
{
    ExplorationSpeechBubbles::ExplorationSpeechBubbles(const content::WorldContent::Presentation::SpeechBubbles& layout,
                                                       const sf::Texture& texture,
                                                       const sf::Font& font,
                                                       gameplay::IRandom& random)
        : layout_(layout)
        , texture_(texture)
        , font_(font)
        , random_(random)
        , untilCheck_(sf::seconds(static_cast<float>(layout.checkIntervalSeconds)))
    {
    }

    void ExplorationSpeechBubbles::update(sf::Time elapsed,
                                           const std::array<AnimationBounds, 4>& speakers,
                                           const std::array<bool, 4>& available,
                                           bool active)
    {
        active_ = active;
        if (active)
        {
            untilCheck_ -= elapsed;
            while (untilCheck_ <= sf::Time::Zero)
            {
                untilCheck_ += sf::seconds(static_cast<float>(layout_.checkIntervalSeconds));
                if (random_.chance(layout_.appearanceChance))
                {
                    createBubble(available);
                }
            }
        }
        for (auto& bubble : bubbles_)
        {
            bubble.remaining -= elapsed;
            positionBubble(bubble, speakers.at(bubble.speaker));
        }
        std::erase_if(bubbles_, [](const Bubble& bubble) { return bubble.remaining <= sf::Time::Zero; });
    }

    void ExplorationSpeechBubbles::render(sf::RenderTarget& target) const
    {
        if (!active_)
        {
            return;
        }
        for (const auto& bubble : bubbles_)
        {
            target.draw(bubble.background);
            target.draw(bubble.text);
        }
    }

    void ExplorationSpeechBubbles::createBubble(const std::array<bool, 4>& available)
    {
        static constexpr std::array<std::string_view, 4> names{"simon", "ole", "anna", "markus"};
        std::vector<std::size_t> candidates;
        for (std::size_t index = 0; index < available.size(); ++index)
        {
            if (available.at(index) &&
                layout_.lines.contains(gameplay::ContentDefinitionId{std::string{"player."} + std::string{names.at(index)}}))
            {
                candidates.push_back(index);
            }
        }
        if (candidates.empty())
        {
            return;
        }
        const auto speaker = candidates.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(candidates.size()) - 1)));
        const auto id = gameplay::ContentDefinitionId{std::string{"player."} + std::string{names.at(speaker)}};
        const auto& lines = layout_.lines.at(id);
        auto text = wrappedText(lines.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(lines.size()) - 1))));
        const auto textBounds = text.getGlobalBounds();
        NinePatch background{texture_};
        const sf::Vector2f size{textBounds.size.x + static_cast<float>(layout_.padding.x),
                               textBounds.size.y + static_cast<float>(layout_.padding.y)};
        background.setSize(size);
        bubbles_.push_back({speaker, std::move(text), std::move(background), sf::seconds(static_cast<float>(layout_.durationSeconds))});
    }

    void ExplorationSpeechBubbles::positionBubble(Bubble& bubble, const AnimationBounds& speaker) const
    {
        const auto backgroundBounds = bubble.background.globalBounds();
        const sf::Vector2f position{speaker.left + speaker.width + static_cast<float>(layout_.speakerOffset.x),
                                   speaker.top - backgroundBounds.size.y + static_cast<float>(layout_.speakerOffset.y)};
        bubble.background.setPosition(position);
        const auto textBounds = bubble.text.getLocalBounds();
        const auto centeredY = position.y + (backgroundBounds.size.y - textBounds.size.y) / 2.0F - textBounds.position.y +
                               static_cast<float>(layout_.textVerticalOffset);
        const auto verticalPadding = static_cast<float>(layout_.padding.y) / 2.0F;
        const auto minimumY = position.y + verticalPadding - textBounds.position.y;
        const auto maximumY = position.y + backgroundBounds.size.y - verticalPadding - textBounds.position.y - textBounds.size.y;
        bubble.text.setPosition({position.x + (backgroundBounds.size.x - textBounds.size.x) / 2.0F - textBounds.position.x,
                                 std::clamp(centeredY, minimumY, std::max(minimumY, maximumY))});
    }

    sf::Text ExplorationSpeechBubbles::wrappedText(const std::string& value) const
    {
        sf::Text result{font_, "", static_cast<unsigned int>(layout_.characterSize)};
        result.setFillColor(sf::Color::Black);
        std::istringstream words{value};
        std::string word;
        std::string line;
        std::string wrapped;
        while (words >> word)
        {
            const auto candidate = line.empty() ? word : line + " " + word;
            result.setString(sf::String::fromUtf8(candidate.begin(), candidate.end()));
            if (!line.empty() && result.getLocalBounds().size.x > static_cast<float>(layout_.maximumTextWidth))
            {
                wrapped += (wrapped.empty() ? "" : "\n") + line;
                line = word;
            }
            else
            {
                line = candidate;
            }
        }
        wrapped += (wrapped.empty() ? "" : "\n") + line;
        result.setString(sf::String::fromUtf8(wrapped.begin(), wrapped.end()));
        return result;
    }
} // namespace soma::presentation
