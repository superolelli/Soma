#include "presentation/sfml/economy/EconomyTooltip.hpp"
#include "presentation/sfml/economy/RuleBuilderPanel.hpp"
#include "presentation/sfml/common/UiCursor.hpp"
#include "gameplay/rules/ComposedRule.hpp"

#include <algorithm>
#include <sstream>

namespace soma::presentation
{
    namespace
    {
        sf::Text wrappedText(const sf::Font& font, std::string_view value, unsigned int size, float maximumWidth)
        {
            sf::Text result{font, "", size};
            std::istringstream words{std::string{value}};
            std::string word;
            std::string line;
            std::string wrapped;
            while (words >> word)
            {
                const auto candidate = line.empty() ? word : line + " " + word;
                result.setString(sf::String::fromUtf8(candidate.begin(), candidate.end()));
                if (!line.empty() && result.getLocalBounds().size.x > maximumWidth)
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
    } // namespace

    void RuleBuilderPanel::render()
    {
        if (drawBackground_)
        {
            drawBackground_();
        }
        else
        {
            window_.clear(sf::Color{35, 28, 24});
        }
        sf::Sprite background{panel_};
        background.setPosition({static_cast<float>(content_.presentation.panelX), static_cast<float>(content_.presentation.panelY)});
        window_.draw(background);
        const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
        sf::Text title{titleFont_, "Spielregeln", static_cast<unsigned int>(content_.presentation.titleSize)};
        title.setFillColor(sf::Color::Black);
        const auto titleBounds = title.getLocalBounds();
        title.setPosition({static_cast<float>(content_.presentation.panelX) + (static_cast<float>(panel_.getSize().x) - titleBounds.size.x) / 2.0F -
                               titleBounds.position.x,
                           static_cast<float>(content_.presentation.panelY + 48) - titleBounds.position.y});
        window_.draw(title);
        drawLibrary(mouse);
        drawRules(mouse);
        if (!message_.empty())
        {
            drawText(message_, {850.0F, 875.0F}, 18U, sf::Color{90, 35, 25});
        }
        if (dragged_)
        {
            sf::RectangleShape preview{{static_cast<float>(content_.presentation.partWidth), 52.0F}};
            preview.setFillColor(color(content_.fragments.at(*dragged_).type, 190));
            preview.setPosition(dragPosition_ - sf::Vector2f{95.0F, 26.0F});
            window_.draw(preview);
            drawText(content_.fragments.at(*dragged_).text, dragPosition_ - sf::Vector2f{88.0F, 16.0F}, 16U);
        }
        else if (const auto hovered = fragmentAt(mouse))
        {
            drawRuleFragmentTooltip(window_, font_, content_.fragments.at(*hovered), mouse);
        }
        sf::Sprite close{closeButton_};
        const auto closeWidth = static_cast<int>(closeButton_.getSize().x / 3U);
        const sf::FloatRect closeBounds{{static_cast<float>(content_.presentation.closeX), static_cast<float>(content_.presentation.closeY)},
                                        {static_cast<float>(closeWidth), static_cast<float>(closeButton_.getSize().y)}};
        close.setTextureRect({{closeBounds.contains(mouse) ? closeWidth : 0, 0}, {closeWidth, static_cast<int>(closeButton_.getSize().y)}});
        close.setPosition(closeBounds.position);
        window_.draw(close);
        if (confirmingUnlock_)
        {
            drawConfirmation(mouse);
        }
        setUiCursor(window_,
                    confirmingUnlock_ || scrollbarDragging_ || dragged_.has_value() || fragmentAt(mouse).has_value() || ruleAt(mouse).has_value() ||
                        closeBounds.contains(mouse) || (navigationHovered_ && navigationHovered_(mouse)));
        window_.display();
    }

    void RuleBuilderPanel::drawConfirmation(sf::Vector2f mouse) const
    {
        const auto& layout = content_.presentation;
        sf::RectangleShape dimmer{{static_cast<float>(layout.referenceWidth), static_cast<float>(layout.referenceHeight)}};
        dimmer.setFillColor(sf::Color{0, 0, 0, 125});
        window_.draw(dimmer);
        const auto panelPosition =
            sf::Vector2f{static_cast<float>(content_.presentation.confirmationX), static_cast<float>(content_.presentation.confirmationY)};
        sf::Sprite panel{confirmationDialog_};
        panel.setPosition(panelPosition);
        window_.draw(panel);
        const auto slot = state_.ruleSlotsUnlocked();
        const auto cost = content_.slotUnlockCosts.at(static_cast<std::size_t>(slot - 1));
        auto question = wrappedText(font_,
                                    "Regelplatz für " + std::to_string(cost) + " Karten freischalten?",
                                    static_cast<unsigned int>(layout.confirmationTextSize),
                                    static_cast<float>(layout.confirmationMessageWidth));
        question.setFillColor(sf::Color::White);
        const auto questionBounds = question.getLocalBounds();
        question.setPosition(
            {panelPosition.x + (static_cast<float>(confirmationDialog_.getSize().x) - questionBounds.size.x) / 2.0F - questionBounds.position.x,
             panelPosition.y + static_cast<float>(content_.presentation.confirmationMessageY) - questionBounds.position.y});
        window_.draw(question);

        const auto buttonWidth = static_cast<int>(confirmationButton_.getSize().x / 3U);
        const auto drawButton = [&](std::string_view label, int x, int y) {
            sf::Sprite button{confirmationButton_};
            button.setPosition(panelPosition + sf::Vector2f{static_cast<float>(x), static_cast<float>(y)});
            const auto hovered =
                sf::FloatRect{button.getPosition(), {static_cast<float>(buttonWidth), static_cast<float>(confirmationButton_.getSize().y)}}.contains(
                    mouse);
            button.setTextureRect({{hovered ? buttonWidth : 0, 0}, {buttonWidth, static_cast<int>(confirmationButton_.getSize().y)}});
            window_.draw(button);
            sf::Text text{font_, sf::String::fromUtf8(label.begin(), label.end()), static_cast<unsigned int>(layout.confirmationButtonTextSize)};
            text.setFillColor(sf::Color::Black);
            const auto bounds = text.getLocalBounds();
            text.setPosition(button.getGlobalBounds().getCenter() - bounds.position - bounds.size / 2.0F);
            window_.draw(text);
        };
        drawButton("Ja", content_.presentation.confirmationYesX, content_.presentation.confirmationYesY);
        drawButton("Nein", content_.presentation.confirmationNoX, content_.presentation.confirmationNoY);
    }

    void RuleBuilderPanel::drawLibrary(sf::Vector2f mouse) const
    {
        const auto entries = library();
        for (auto visible = 0; visible < content_.presentation.visibleFragmentRows; ++visible)
        {
            const auto index = scroll_ + visible;
            if (index >= static_cast<int>(entries.size()))
            {
                break;
            }
            const auto& entry = entries.at(static_cast<std::size_t>(index));
            const auto bounds = libraryBounds(static_cast<std::size_t>(visible));
            sf::Sprite background{fragmentBackground_};
            background.setPosition(bounds.position);
            if (bounds.contains(mouse))
            {
                background.setColor(sf::Color{255, 240, 205});
            }
            window_.draw(background);
            sf::Sprite sigil{sigils_.at(entry.definition->type)};
            sigil.setPosition(bounds.position + sf::Vector2f{static_cast<float>(content_.presentation.sigilOffsetX),
                                                             static_cast<float>(content_.presentation.sigilOffsetY)});
            window_.draw(sigil);
            drawText(entry.definition->text,
                     bounds.position + sf::Vector2f{static_cast<float>(content_.presentation.fragmentTextOffsetX),
                                                    static_cast<float>(content_.presentation.fragmentTextOffsetY)},
                     17U);
            drawText("x" + std::to_string(entry.count),
                     bounds.position + sf::Vector2f{static_cast<float>(content_.presentation.fragmentCountOffsetX),
                                                    static_cast<float>(content_.presentation.fragmentCountOffsetY)},
                     17U);
        }
        const auto hiddenRows = std::max(0, static_cast<int>(entries.size()) - content_.presentation.visibleFragmentRows);
        if (hiddenRows > 0)
        {
            const auto bounds = scrollbarThumb(entries);
            sf::RectangleShape thumb{bounds.size};
            thumb.setFillColor(sf::Color{105, 105, 105});
            thumb.setPosition(bounds.position);
            window_.draw(thumb);
        }
    }

    void RuleBuilderPanel::drawRules(sf::Vector2f mouse) const
    {
        for (std::size_t slot = 0; slot < gameplay::maximumRuleSlots; ++slot)
        {
            if (slot >= static_cast<std::size_t>(state_.ruleSlotsUnlocked()))
            {
                const auto next = slot == static_cast<std::size_t>(state_.ruleSlotsUnlocked());
                sf::Sprite lock{slotLock_};
                lock.setPosition({static_cast<float>(content_.presentation.lockX),
                                  static_cast<float>(content_.presentation.lockY + static_cast<int>(slot) * content_.presentation.ruleRowSpacing)});
                window_.draw(lock);
                if (next)
                {
                    sf::Text cost{font_, std::to_string(content_.slotUnlockCosts.at(slot - 1U)), 22U};
                    cost.setFillColor(sf::Color::White);
                    const auto textBounds = cost.getLocalBounds();
                    const sf::FloatRect plaque{
                        lock.getPosition() +
                            sf::Vector2f{static_cast<float>(content_.presentation.lockPriceX), static_cast<float>(content_.presentation.lockPriceY)},
                        {static_cast<float>(content_.presentation.lockPriceWidth), static_cast<float>(content_.presentation.lockPriceHeight)}};
                    cost.setPosition(plaque.getCenter() - textBounds.position - textBounds.size / 2.0F);
                    window_.draw(cost);
                }
                continue;
            }
            for (const auto type : visibleTypes(slot))
            {
                drawPart(slot, type, mouse);
            }
            auto sentence = wrappedText(font_, ruleSentence(slot), 18U, static_cast<float>(content_.presentation.ruleContentWidth));
            sentence.setFillColor(sf::Color{38, 25, 18});
            const auto sentenceBounds = sentence.getLocalBounds();
            const sf::Vector2f area{static_cast<float>(content_.presentation.ruleX),
                                    static_cast<float>(content_.presentation.ruleY + static_cast<int>(slot) * content_.presentation.ruleRowSpacing +
                                                       content_.presentation.sentenceOffsetY)};
            sentence.setPosition(
                {area.x + (static_cast<float>(content_.presentation.ruleContentWidth) - sentenceBounds.size.x) / 2.0F - sentenceBounds.position.x,
                 area.y - sentenceBounds.position.y});
            window_.draw(sentence);
        }
    }

    void RuleBuilderPanel::drawPart(std::size_t slot, content::RuleFragmentType type, sf::Vector2f mouse) const
    {
        const auto bounds = partBounds(slot, type);
        const auto& rule = state_.rules().at(slot);
        const std::array parts{&rule.trigger, &rule.subject, &rule.effect, &rule.modifier};
        const auto& part = *parts.at(static_cast<std::size_t>(type));
        const auto compatible = !dragged_ || content_.fragments.at(*dragged_).type != type || service_.accepts(slot, *dragged_);
        sf::RectangleShape box{bounds.size};
        box.setPosition(bounds.position);
        box.setFillColor(color(type, compatible ? (bounds.contains(mouse) ? 235 : 165) : 65));
        box.setOutlineColor(sf::Color{95, 55, 30});
        box.setOutlineThickness(2.0F);
        window_.draw(box);
        auto text = wrappedText(font_, part ? content_.fragments.at(*part).text : "leer", 16U, bounds.size.x - 16.0F);
        text.setFillColor(sf::Color::Black);
        const auto textBounds = text.getLocalBounds();
        text.setPosition({bounds.position.x + (bounds.size.x - textBounds.size.x) / 2.0F - textBounds.position.x,
                          bounds.position.y + (bounds.size.y - textBounds.size.y) / 2.0F - textBounds.position.y});
        window_.draw(text);
    }

    void RuleBuilderPanel::drawText(std::string_view value, sf::Vector2f position, unsigned int size, sf::Color textColor) const
    {
        sf::Text text{font_, sf::String::fromUtf8(value.begin(), value.end()), size};
        text.setFillColor(textColor);
        text.setPosition(position);
        window_.draw(text);
    }

    sf::Color RuleBuilderPanel::color(content::RuleFragmentType type, std::uint8_t alpha) const
    {
        switch (type)
        {
        case content::RuleFragmentType::trigger:
            return {224, 194, 52, alpha};
        case content::RuleFragmentType::subject:
            return {76, 146, 183, alpha};
        case content::RuleFragmentType::effect:
            return {78, 166, 83, alpha};
        case content::RuleFragmentType::modifier:
            return {135, 87, 157, alpha};
        }
        return sf::Color::White;
    }
} // namespace soma::presentation
