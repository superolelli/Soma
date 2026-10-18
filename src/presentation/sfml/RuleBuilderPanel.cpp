#include "presentation/sfml/RuleBuilderPanel.hpp"

#include <algorithm>

namespace soma::presentation
{
    RuleBuilderPanel::RuleBuilderPanel(sf::RenderWindow& window,
                                       const content::RuleContent& content,
                                       game::CampaignState& state,
                                       const sf::Font& font,
                                       const sf::Font& titleFont,
                                       std::function<void()> drawBackground,
                                       std::function<bool(sf::Vector2f)> navigate,
                                       std::function<bool(sf::Vector2f)> navigationHovered)
        : window_(window)
        , content_(content)
        , state_(state)
        , service_(state, content)
        , font_(font)
        , titleFont_(titleFont)
        , drawBackground_(std::move(drawBackground))
        , navigate_(std::move(navigate))
        , navigationHovered_(std::move(navigationHovered))
        , panel_(content.panel)
        , fragmentBackground_(content.fragmentBackground)
        , slotLock_(content.slotLock)
        , closeButton_(content.closeButton)
        , confirmationDialog_(content.confirmationDialog)
        , confirmationButton_(content.confirmationButton)
    {
        for (const auto& [type, path] : content.fragmentSigils)
        {
            sigils_.emplace(type, sf::Texture{path});
        }
    }

    void RuleBuilderPanel::processEvents()
    {
        while (const auto event = window_.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window_.close();
                closeRequested_ = true;
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape)
            {
                if (confirmingUnlock_)
                {
                    confirmingUnlock_ = false;
                }
                else
                {
                    closeRequested_ = true;
                }
            }
            if (confirmingUnlock_)
            {
                if (const auto* release = event->getIf<sf::Event::MouseButtonReleased>(); release && release->button == sf::Mouse::Button::Left)
                {
                    static_cast<void>(handleConfirmationClick(window_.mapPixelToCoords(release->position)));
                }
                continue;
            }
            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                const auto maximum = std::max(0, static_cast<int>(library().size()) - content_.presentation.visibleFragmentRows);
                scroll_ = std::clamp(scroll_ - static_cast<int>(wheel->delta), 0, maximum);
            }
            if (const auto* moved = event->getIf<sf::Event::MouseMoved>())
            {
                handleMouseMove(*moved);
            }
            if (const auto* press = event->getIf<sf::Event::MouseButtonPressed>())
            {
                handleMousePress(*press);
            }
            if (const auto* release = event->getIf<sf::Event::MouseButtonReleased>(); release && release->button == sf::Mouse::Button::Left)
            {
                handleMouseRelease(*release);
            }
        }
    }

    void RuleBuilderPanel::handleMouseMove(const sf::Event::MouseMoved& event)
    {
        const auto position = window_.mapPixelToCoords(event.position);
        if (scrollbarDragging_)
        {
            updateScrollFromMouse(position.y);
        }
        if (dragged_)
        {
            dragPosition_ = position;
        }
    }

    void RuleBuilderPanel::handleMousePress(const sf::Event::MouseButtonPressed& event)
    {
        const auto position = window_.mapPixelToCoords(event.position);
        if (event.button == sf::Mouse::Button::Right)
        {
            if (const auto part = partAt(position))
            {
                static_cast<void>(service_.remove(part->first, part->second));
            }
            return;
        }
        if (event.button != sf::Mouse::Button::Left)
        {
            return;
        }

        const auto thumb = scrollbarThumb(library());
        const sf::FloatRect track{
            {static_cast<float>(content_.presentation.scrollbarX), static_cast<float>(content_.presentation.scrollbarY)},
            {static_cast<float>(content_.presentation.scrollbarWidth), static_cast<float>(content_.presentation.scrollbarHeight)}};
        if (thumb.size.x > 0.0F && (thumb.contains(position) || track.contains(position)))
        {
            scrollbarDragging_ = true;
            scrollbarDragOffset_ = thumb.contains(position) ? position.y - thumb.position.y : thumb.size.y / 2.0F;
            if (!thumb.contains(position))
            {
                updateScrollFromMouse(position.y);
            }
            return;
        }
        beginFragmentDrag(position);
    }

    void RuleBuilderPanel::beginFragmentDrag(sf::Vector2f position)
    {
        draggedFromPart_.reset();
        dragged_ = fragmentAt(position);
        if (!dragged_)
        {
            draggedFromPart_ = partAt(position);
            if (draggedFromPart_)
            {
                dragged_ = fragmentInPart(*draggedFromPart_);
            }
        }
        dragPosition_ = position;
        if (!dragged_)
        {
            tryOpenSlotUnlock(position);
        }
    }

    void RuleBuilderPanel::tryOpenSlotUnlock(sf::Vector2f position)
    {
        const auto slot = state_.ruleSlotsUnlocked();
        if (slot >= static_cast<int>(game::maximumRuleSlots))
        {
            return;
        }
        const sf::FloatRect lockBounds{{static_cast<float>(content_.presentation.lockX),
                                        static_cast<float>(content_.presentation.lockY + slot * content_.presentation.ruleRowSpacing)},
                                       {static_cast<float>(slotLock_.getSize().x), static_cast<float>(slotLock_.getSize().y)}};
        confirmingUnlock_ = lockBounds.contains(position);
    }

    void RuleBuilderPanel::handleMouseRelease(const sf::Event::MouseButtonReleased& event)
    {
        scrollbarDragging_ = false;
        const auto position = window_.mapPixelToCoords(event.position);
        sf::Sprite close{closeButton_};
        close.setTextureRect({{0, 0}, {static_cast<int>(closeButton_.getSize().x / 3U), static_cast<int>(closeButton_.getSize().y)}});
        close.setPosition({static_cast<float>(content_.presentation.closeX), static_cast<float>(content_.presentation.closeY)});
        if (close.getGlobalBounds().contains(position) || (navigate_ && navigate_(position)))
        {
            closeRequested_ = true;
        }
        else
        {
            finishFragmentDrag(position);
        }
        dragged_.reset();
        draggedFromPart_.reset();
    }

    void RuleBuilderPanel::finishFragmentDrag(sf::Vector2f position)
    {
        if (!dragged_)
        {
            return;
        }
        if (const auto target = ruleAt(position))
        {
            if (!draggedFromPart_ || draggedFromPart_->first != *target)
            {
                if (draggedFromPart_)
                {
                    static_cast<void>(service_.remove(draggedFromPart_->first, draggedFromPart_->second));
                }
                const auto result = service_.assign(*target, *dragged_);
                if (!result.hasValue() && draggedFromPart_)
                {
                    static_cast<void>(service_.assign(draggedFromPart_->first, *dragged_));
                }
                message_ = result.hasValue() ? "Regelteil eingesetzt" : result.error();
            }
            const auto maximum = std::max(0, static_cast<int>(library().size()) - content_.presentation.visibleFragmentRows);
            scroll_ = std::min(scroll_, maximum);
            return;
        }
        if (draggedFromPart_ && fragmentPoolBounds().contains(position))
        {
            static_cast<void>(service_.remove(draggedFromPart_->first, draggedFromPart_->second));
            message_ = "Regelteil zurückgelegt";
        }
    }

    sf::FloatRect RuleBuilderPanel::fragmentPoolBounds() const
    {
        return {{static_cast<float>(content_.presentation.fragmentListX), static_cast<float>(content_.presentation.fragmentListY)},
                {static_cast<float>(content_.presentation.fragmentListWidth),
                 static_cast<float>(content_.presentation.visibleFragmentRows * content_.presentation.fragmentRowHeight)}};
    }

    bool RuleBuilderPanel::closeRequested() const noexcept
    {
        return closeRequested_;
    }

} // namespace soma::presentation
