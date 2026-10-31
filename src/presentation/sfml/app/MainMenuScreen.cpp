#include "presentation/sfml/app/MainMenuScreen.hpp"

#include "presentation/sfml/common/AudioSettings.hpp"
#include "presentation/sfml/common/UiCursor.hpp"

#include <algorithm>

namespace soma::presentation
{
    namespace
    {
        sf::Text centeredText(const sf::Font& font, const sf::String& value, unsigned int size, sf::Vector2f center)
        {
            sf::Text text{font, value, size};
            text.setFillColor(sf::Color::White);
            text.setOutlineColor(sf::Color::Black);
            text.setOutlineThickness(2.0F);
            const auto bounds = text.getLocalBounds();
            text.setPosition({center.x - bounds.position.x - bounds.size.x / 2.0F,
                              center.y - bounds.position.y - bounds.size.y / 2.0F});
            return text;
        }

        sf::Vector2f vector(const content::Point& point)
        {
            return {static_cast<float>(point.x), static_cast<float>(point.y)};
        }
    }

    MainMenuScreen::MainMenuScreen(sf::RenderWindow& window,
                                   const content::GameFlowContent& content,
                                   const persistence::SaveSlotService& slots)
        : window_(window)
        , content_(content)
        , slots_(slots)
        , background_(content.assets.at("background"))
        , savePanel_(content.assets.at("save_panel"))
        , nameInputPanel_(content.assets.at("name_input"))
        , button_(content.assets.at("button"))
        , closeButton_(content.assets.at("close_button"))
        , font_(content.assets.at("font"))
        , music_(content.mainRoom.music)
    {
        refreshSlots();
        music_->setLooping(true);
        music_->setRelativeToListener(true);
        music_->setVolume(AudioSettings::music(static_cast<float>(content.mainRoom.musicVolume)));
        music_->play();
    }

    std::optional<MainMenuSelection> MainMenuScreen::run()
    {
        while (window_.isOpen() && !selection_)
        {
            while (const auto event = window_.pollEvent())
            {
                processEvent(*event);
            }
            render();
        }
        return selection_;
    }

    void MainMenuScreen::processEvent(const sf::Event& event)
    {
        if (event.is<sf::Event::Closed>())
        {
            window_.close();
            return;
        }
        if (const auto* key = event.getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Escape)
            {
                if (enteringName_)
                {
                    enteringName_ = false;
                    enteredName_.clear();
                }
                else
                {
                    window_.close();
                }
            }
            else if (enteringName_ && key->code == sf::Keyboard::Key::Enter)
            {
                submitName();
            }
        }
        if (const auto* text = event.getIf<sf::Event::TextEntered>(); text && enteringName_)
        {
            handleNameInput(text->unicode);
        }
        if (const auto* click = event.getIf<sf::Event::MouseButtonReleased>();
            click && click->button == sf::Mouse::Button::Left)
        {
            mousePressed_ = false;
            handleClick(window_.mapPixelToCoords(click->position));
        }
        if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>(); press && press->button == sf::Mouse::Button::Left)
        {
            mousePressed_ = true;
        }
    }

    void MainMenuScreen::handleClick(sf::Vector2f position)
    {
        if (enteringName_)
        {
            if (buttonBounds(vector(content_.menu.nameButtonPosition)).contains(position))
            {
                submitName();
            }
            return;
        }

        const auto closeSize = sf::Vector2f{static_cast<float>(closeButton_.getSize().x / 3U),
                                            static_cast<float>(closeButton_.getSize().y)};
        if (sf::FloatRect{vector(content_.menu.closeButtonPosition), closeSize}.contains(position))
        {
            window_.close();
            return;
        }
        for (const auto& slot : slotSummaries_)
        {
            const auto panelPosition = vector(content_.menu.savePanelPositions.at(static_cast<std::size_t>(slot.index)));
            const sf::FloatRect panelBounds{panelPosition,
                                            {static_cast<float>(savePanel_.getSize().x), static_cast<float>(savePanel_.getSize().y)}};
            if (panelBounds.contains(position))
            {
                activeSlot_ = slot.index;
                feedback_.clear();
                return;
            }
        }
        if (!activeSlot_)
        {
            return;
        }
        const auto& slot = slotSummaries_.at(static_cast<std::size_t>(*activeSlot_));
        const auto panel = vector(content_.menu.savePanelPositions.at(static_cast<std::size_t>(*activeSlot_)));
        const auto firstAction = sf::Vector2f{panel.x + (static_cast<float>(savePanel_.getSize().x) - buttonBounds({}).size.x) / 2.0F,
                                              panel.y + static_cast<float>(content_.menu.firstActionOffsetY)};
        if (buttonBounds(firstAction).contains(position))
        {
            if (slot.state == persistence::SaveSlotState::empty)
            {
                enteringName_ = true;
                enteredName_.clear();
            }
            else if (slot.state == persistence::SaveSlotState::occupied)
            {
                const auto loaded = slots_.load(*activeSlot_);
                if (loaded.hasValue()) selection_ = MainMenuSelection{*activeSlot_, loaded.value()};
                else feedback_ = loaded.error();
            }
            else
            {
                feedback_ = slot.error;
            }
            return;
        }
        const auto secondAction = firstAction + sf::Vector2f{0.0F, static_cast<float>(content_.menu.actionSpacingY)};
        if (slot.state != persistence::SaveSlotState::empty && buttonBounds(secondAction).contains(position))
        {
            const auto removed = slots_.remove(*activeSlot_);
            feedback_ = removed.hasValue() ? "Spielstand gelöscht" : removed.error();
            activeSlot_.reset();
            refreshSlots();
        }
    }

    void MainMenuScreen::handleNameInput(char32_t character)
    {
        if (character == 8)
        {
            if (!enteredName_.isEmpty())
            {
                enteredName_.erase(enteredName_.getSize() - 1);
            }
            return;
        }
        if (character >= 32 && character != 127 && enteredName_.getSize() < static_cast<std::size_t>(content_.menu.maximumNameCharacters))
        {
            enteredName_ += character;
        }
    }

    void MainMenuScreen::submitName()
    {
        if (!activeSlot_ || enteredName_.isEmpty())
        {
            feedback_ = "Bitte einen Namen eingeben";
            return;
        }
        const auto utf8 = enteredName_.toUtf8();
        const std::string name{utf8.begin(), utf8.end()};
        const auto created = slots_.create(*activeSlot_, name);
        if (!created.hasValue())
        {
            feedback_ = created.error();
            return;
        }
        selection_ = MainMenuSelection{*activeSlot_, created.value()};
    }

    void MainMenuScreen::refreshSlots()
    {
        slotSummaries_ = slots_.discover();
    }

    sf::FloatRect MainMenuScreen::buttonBounds(sf::Vector2f position) const
    {
        return {position, {static_cast<float>(button_.getSize().x / 3U), static_cast<float>(button_.getSize().y)}};
    }

    bool MainMenuScreen::clickableAt(sf::Vector2f position) const
    {
        if (enteringName_) return buttonBounds(vector(content_.menu.nameButtonPosition)).contains(position);
        const auto closeSize = sf::Vector2f{static_cast<float>(closeButton_.getSize().x / 3U),
                                            static_cast<float>(closeButton_.getSize().y)};
        if (sf::FloatRect{vector(content_.menu.closeButtonPosition), closeSize}.contains(position)) return true;
        for (const auto& slot : slotSummaries_)
        {
            const auto panel = vector(content_.menu.savePanelPositions.at(static_cast<std::size_t>(slot.index)));
            if (sf::FloatRect{panel, sf::Vector2f{savePanel_.getSize()}}.contains(position)) return true;
        }
        if (!activeSlot_) return false;
        const auto panel = vector(content_.menu.savePanelPositions.at(static_cast<std::size_t>(*activeSlot_)));
        const auto first = sf::Vector2f{panel.x + (static_cast<float>(savePanel_.getSize().x) - buttonBounds({}).size.x) / 2.0F,
                                        panel.y + static_cast<float>(content_.menu.firstActionOffsetY)};
        if (buttonBounds(first).contains(position)) return true;
        const auto& slot = slotSummaries_.at(static_cast<std::size_t>(*activeSlot_));
        return slot.state != persistence::SaveSlotState::empty &&
               buttonBounds(first + sf::Vector2f{0.0F, static_cast<float>(content_.menu.actionSpacingY)}).contains(position);
    }

    void MainMenuScreen::drawButton(sf::Vector2f position, std::string_view label, sf::Vector2f mouse) const
    {
        const auto bounds = buttonBounds(position);
        sf::Sprite sprite{button_};
        const auto frameWidth = static_cast<int>(button_.getSize().x / 3U);
        const auto frame = bounds.contains(mouse) ? (mousePressed_ ? 2 : 1) : 0;
        sprite.setTextureRect({{frame * frameWidth, 0}, {frameWidth, static_cast<int>(button_.getSize().y)}});
        sprite.setPosition(position);
        window_.draw(sprite);
        const sf::String text = sf::String::fromUtf8(label.begin(), label.end());
        window_.draw(centeredText(font_, text, static_cast<unsigned int>(content_.menu.buttonTextSize), bounds.getCenter()));
    }

    void MainMenuScreen::drawSlot(const persistence::SaveSlotSummary& slot) const
    {
        const auto position = vector(content_.menu.savePanelPositions.at(static_cast<std::size_t>(slot.index)));
        sf::Sprite panel{savePanel_};
        panel.setPosition(position);
        window_.draw(panel);
        const auto label = slot.state == persistence::SaveSlotState::empty
                               ? sf::String{L"Leer"}
                               : slot.state == persistence::SaveSlotState::invalid
                                     ? sf::String{L"Beschädigt"}
                                     : sf::String::fromUtf8(slot.playerName.begin(), slot.playerName.end());
        window_.draw(centeredText(font_, label, static_cast<unsigned int>(content_.menu.panelTextSize),
                                  position + sf::Vector2f{savePanel_.getSize().x / 2.0F, savePanel_.getSize().y / 2.0F}));
    }

    void MainMenuScreen::render()
    {
        window_.clear(sf::Color::Black);
        window_.draw(sf::Sprite{background_});
        const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
        setUiCursor(window_, clickableAt(mouse));
        for (const auto& slot : slotSummaries_) drawSlot(slot);

        sf::Sprite close{closeButton_};
        const auto closeWidth = closeButton_.getSize().x / 3U;
        close.setTextureRect({{0, 0}, {static_cast<int>(closeWidth), static_cast<int>(closeButton_.getSize().y)}});
        close.setPosition(vector(content_.menu.closeButtonPosition));
        window_.draw(close);

        if (activeSlot_ && !enteringName_)
        {
            const auto& slot = slotSummaries_.at(static_cast<std::size_t>(*activeSlot_));
            const auto panel = vector(content_.menu.savePanelPositions.at(static_cast<std::size_t>(*activeSlot_)));
            const auto first = sf::Vector2f{panel.x + (static_cast<float>(savePanel_.getSize().x) - buttonBounds({}).size.x) / 2.0F,
                                            panel.y + static_cast<float>(content_.menu.firstActionOffsetY)};
            drawButton(first, slot.state == persistence::SaveSlotState::empty ? "Neues Spiel" :
                              slot.state == persistence::SaveSlotState::occupied ? "Fortsetzen" : "Fehler", mouse);
            if (slot.state != persistence::SaveSlotState::empty)
            {
                drawButton(first + sf::Vector2f{0.0F, static_cast<float>(content_.menu.actionSpacingY)}, "Löschen", mouse);
            }
        }
        if (enteringName_)
        {
            sf::RectangleShape shade{{static_cast<float>(content_.referenceSize.x), static_cast<float>(content_.referenceSize.y)}};
            shade.setFillColor(sf::Color{0, 0, 0, 150});
            window_.draw(shade);
            sf::Sprite inputPanel{nameInputPanel_};
            inputPanel.setPosition(vector(content_.menu.nameInputPosition));
            window_.draw(inputPanel);
            const auto shownName = enteredName_.isEmpty() ? sf::String{L"Name eingeben"} : enteredName_;
            window_.draw(centeredText(font_, shownName, static_cast<unsigned int>(content_.menu.nameTextSize),
                                      vector(content_.menu.nameTextCenter)));
            drawButton(vector(content_.menu.nameButtonPosition), "Fertig", mouse);
        }
        if (!feedback_.empty())
        {
            const auto feedback = sf::String::fromUtf8(feedback_.begin(), feedback_.end());
            window_.draw(centeredText(font_,
                                      feedback,
                                      static_cast<unsigned int>(content_.menu.feedbackTextSize),
                                      vector(content_.menu.feedbackTextCenter)));
        }
        window_.display();
    }
} // namespace soma::presentation
