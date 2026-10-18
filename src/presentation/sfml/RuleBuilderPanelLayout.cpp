#include "presentation/sfml/RuleBuilderPanel.hpp"
#include "presentation/text/RuleDescriptionBuilder.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <ranges>

namespace soma::presentation
{
    namespace
    {
        constexpr std::array fragmentTypes{content::RuleFragmentType::trigger,
                                           content::RuleFragmentType::subject,
                                           content::RuleFragmentType::effect,
                                           content::RuleFragmentType::modifier};
    } // namespace

    std::vector<RuleBuilderPanel::LibraryEntry> RuleBuilderPanel::library() const
    {
        std::vector<LibraryEntry> result;
        for (const auto& [id, definition] : content_.fragments)
        {
            const auto count = service_.availableCount(id);
            if (count > 0)
            {
                result.push_back({&definition, count});
            }
        }
        std::ranges::sort(result, [](const auto& left, const auto& right) {
            if (left.definition->type != right.definition->type)
            {
                return left.definition->type < right.definition->type;
            }
            return left.definition->name < right.definition->name;
        });
        return result;
    }

    sf::FloatRect RuleBuilderPanel::libraryBounds(std::size_t visibleIndex) const
    {
        return {{static_cast<float>(content_.presentation.fragmentListX),
                 static_cast<float>(content_.presentation.fragmentListY + static_cast<int>(visibleIndex) * content_.presentation.fragmentRowHeight)},
                {static_cast<float>(content_.presentation.fragmentListWidth), static_cast<float>(content_.presentation.fragmentRowHeight - 6)}};
    }

    sf::FloatRect RuleBuilderPanel::partBounds(std::size_t slot, content::RuleFragmentType type) const
    {
        const auto types = visibleTypes(slot);
        const auto found = std::ranges::find(types, type);
        if (found == types.end())
        {
            return {};
        }
        const auto column = static_cast<int>(std::distance(types.begin(), found));
        const auto count = static_cast<int>(types.size());
        const auto width = (content_.presentation.ruleContentWidth - (count - 1) * content_.presentation.partSpacing) / count;
        return {{static_cast<float>(content_.presentation.ruleX + column * (width + content_.presentation.partSpacing)),
                 static_cast<float>(content_.presentation.ruleY + static_cast<int>(slot) * content_.presentation.ruleRowSpacing)},
                {static_cast<float>(width), static_cast<float>(content_.presentation.partHeight)}};
    }

    std::vector<content::RuleFragmentType> RuleBuilderPanel::visibleTypes(std::size_t slot) const
    {
        const auto mode = service_.mode(slot);
        if (mode == content::RuleMode::triggered)
        {
            return {content::RuleFragmentType::trigger, content::RuleFragmentType::subject, content::RuleFragmentType::effect};
        }
        if (mode == content::RuleMode::standing)
        {
            return {content::RuleFragmentType::subject, content::RuleFragmentType::modifier};
        }
        return {fragmentTypes.begin(), fragmentTypes.end()};
    }

    std::optional<gameplay::RuleFragmentId> RuleBuilderPanel::fragmentAt(sf::Vector2f position) const
    {
        const auto entries = library();
        for (auto visible = 0; visible < content_.presentation.visibleFragmentRows; ++visible)
        {
            const auto index = scroll_ + visible;
            if (index < static_cast<int>(entries.size()) && libraryBounds(static_cast<std::size_t>(visible)).contains(position))
            {
                return entries.at(static_cast<std::size_t>(index)).definition->id;
            }
        }
        return std::nullopt;
    }

    std::optional<std::pair<std::size_t, content::RuleFragmentType>> RuleBuilderPanel::partAt(sf::Vector2f position) const
    {
        for (std::size_t slot = 0; slot < static_cast<std::size_t>(state_.ruleSlotsUnlocked()); ++slot)
        {
            for (const auto type : visibleTypes(slot))
            {
                if (partBounds(slot, type).contains(position))
                {
                    return std::pair{slot, type};
                }
            }
        }
        return std::nullopt;
    }

    std::optional<gameplay::RuleFragmentId> RuleBuilderPanel::fragmentInPart(const std::pair<std::size_t, content::RuleFragmentType>& part) const
    {
        const auto& rule = state_.rules().at(part.first);
        switch (part.second)
        {
        case content::RuleFragmentType::trigger:
            return rule.trigger;
        case content::RuleFragmentType::subject:
            return rule.subject;
        case content::RuleFragmentType::effect:
            return rule.effect;
        case content::RuleFragmentType::modifier:
            return rule.modifier;
        }
        return std::nullopt;
    }

    std::optional<std::size_t> RuleBuilderPanel::ruleAt(sf::Vector2f position) const
    {
        for (std::size_t slot = 0; slot < static_cast<std::size_t>(state_.ruleSlotsUnlocked()); ++slot)
        {
            const sf::FloatRect bounds{
                {static_cast<float>(content_.presentation.ruleX),
                 static_cast<float>(content_.presentation.ruleY + static_cast<int>(slot) * content_.presentation.ruleRowSpacing)},
                {static_cast<float>(content_.presentation.ruleContentWidth), static_cast<float>(content_.presentation.partHeight)}};
            if (bounds.contains(position))
            {
                return slot;
            }
        }
        return std::nullopt;
    }

    sf::FloatRect RuleBuilderPanel::scrollbarThumb(const std::vector<LibraryEntry>& entries) const
    {
        const auto visible = content_.presentation.visibleFragmentRows;
        if (static_cast<int>(entries.size()) <= visible)
        {
            return {};
        }
        const auto trackHeight = static_cast<float>(content_.presentation.scrollbarHeight);
        const auto thumbHeight = trackHeight * static_cast<float>(visible) / static_cast<float>(entries.size());
        const auto hidden = static_cast<int>(entries.size()) - visible;
        const auto progress = static_cast<float>(std::clamp(scroll_, 0, hidden)) / static_cast<float>(hidden);
        return {{static_cast<float>(content_.presentation.scrollbarX),
                 static_cast<float>(content_.presentation.scrollbarY) + progress * (trackHeight - thumbHeight)},
                {static_cast<float>(content_.presentation.scrollbarWidth), thumbHeight}};
    }

    void RuleBuilderPanel::updateScrollFromMouse(float mouseY)
    {
        const auto entries = library();
        const auto thumb = scrollbarThumb(entries);
        const auto hidden = static_cast<int>(entries.size()) - content_.presentation.visibleFragmentRows;
        if (hidden <= 0 || thumb.size.y >= static_cast<float>(content_.presentation.scrollbarHeight))
        {
            return;
        }
        const auto travel = static_cast<float>(content_.presentation.scrollbarHeight) - thumb.size.y;
        const auto progress = std::clamp((mouseY - scrollbarDragOffset_ - static_cast<float>(content_.presentation.scrollbarY)) / travel, 0.0F, 1.0F);
        scroll_ = static_cast<int>(std::round(progress * static_cast<float>(hidden)));
    }

    std::string RuleBuilderPanel::ruleSentence(std::size_t slot) const
    {
        return buildGermanRuleDescription(state_.rules().at(slot), service_.mode(slot), content_);
    }

    bool RuleBuilderPanel::handleConfirmationClick(sf::Vector2f position)
    {
        const auto buttonWidth = static_cast<float>(confirmationButton_.getSize().x / 3U);
        const auto buttonSize = sf::Vector2f{buttonWidth, static_cast<float>(confirmationButton_.getSize().y)};
        const auto panel =
            sf::Vector2f{static_cast<float>(content_.presentation.confirmationX), static_cast<float>(content_.presentation.confirmationY)};
        const sf::FloatRect yes{panel + sf::Vector2f{static_cast<float>(content_.presentation.confirmationYesX),
                                                     static_cast<float>(content_.presentation.confirmationYesY)},
                                buttonSize};
        const sf::FloatRect no{panel + sf::Vector2f{static_cast<float>(content_.presentation.confirmationNoX),
                                                    static_cast<float>(content_.presentation.confirmationNoY)},
                               buttonSize};
        if (yes.contains(position))
        {
            const auto result = service_.unlockNextSlot();
            message_ = result.hasValue() ? "Regelplatz freigeschaltet" : result.error();
            confirmingUnlock_ = false;
            return true;
        }
        if (no.contains(position))
        {
            confirmingUnlock_ = false;
            return true;
        }
        return false;
    }

} // namespace soma::presentation
