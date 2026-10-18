#include "presentation/sfml/RuleBuilderPanel.hpp"

#include "presentation/sfml/EconomyTooltip.hpp"
#include "presentation/sfml/UiCursor.hpp"
#include "presentation/text/RuleDescriptionBuilder.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace soma::presentation
{
    namespace
    {
        constexpr std::array fragmentTypes{content::RuleFragmentType::trigger,
                                           content::RuleFragmentType::subject,
                                           content::RuleFragmentType::effect,
                                           content::RuleFragmentType::modifier};

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
        const sf::FloatRect lockBounds{
            {static_cast<float>(content_.presentation.lockX),
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

    bool RuleBuilderPanel::closeRequested() const noexcept
    {
        return closeRequested_;
    }

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
            sf::Text text{font_, sf::String::fromUtf8(label.begin(), label.end()),
                          static_cast<unsigned int>(layout.confirmationButtonTextSize)};
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
        for (std::size_t slot = 0; slot < game::maximumRuleSlots; ++slot)
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
