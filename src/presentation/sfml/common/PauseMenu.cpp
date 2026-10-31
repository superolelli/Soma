#include "presentation/sfml/common/PauseMenu.hpp"

#include "presentation/sfml/common/AudioSettings.hpp"
#include "presentation/sfml/battle/BattlePlaybackSettings.hpp"
#include "presentation/sfml/common/UiCursor.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation
{
    namespace
    {
        sf::Text centered(const sf::Font& font, const sf::String& value, unsigned int size, sf::Vector2f center)
        {
            sf::Text text{font, value, size};
            text.setFillColor(sf::Color::White);
            text.setOutlineColor(sf::Color::Black);
            text.setOutlineThickness(2.0F);
            const auto bounds = text.getLocalBounds();
            text.setPosition(center - bounds.position - bounds.size / 2.0F);
            return text;
        }

        sf::Vector2f vector(const content::Point& point)
        {
            return {static_cast<float>(point.x), static_cast<float>(point.y)};
        }
    } // namespace

    PauseMenu::PauseMenu(const content::GameFlowContent& content, PauseMenuContext context)
        : content_(content), context_(context), button_(content.assets.at("button")), font_(content.assets.at("font"))
    {
    }

    void PauseMenu::open()
    {
        open_ = true;
        settings_ = false;
    }

    bool PauseMenu::isOpen() const noexcept
    {
        return open_;
    }

    std::optional<PauseMenuAction> PauseMenu::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape)
        {
            if (settings_) settings_ = false;
            else
            {
                const auto wasOpen = open_;
                open_ = !open_;
                if (wasOpen) return PauseMenuAction::resume;
            }
            return std::nullopt;
        }
        if (!open_)
        {
            return std::nullopt;
        }
        if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>(); press && press->button == sf::Mouse::Button::Left)
        {
            mousePressed_ = true;
            if (settings_)
            {
                const auto mouse = window.mapPixelToCoords(press->position,
                                                           sf::View{sf::FloatRect{{0.0F, 0.0F}, vector(content_.referenceSize)}});
                if (sliderBounds(SettingsSlider::music).contains(mouse)) draggedSlider_ = SettingsSlider::music;
                else if (sliderBounds(SettingsSlider::sound).contains(mouse)) draggedSlider_ = SettingsSlider::sound;
                else if (sliderBounds(SettingsSlider::battleSpeed).contains(mouse)) draggedSlider_ = SettingsSlider::battleSpeed;
                if (draggedSlider_) updateSlider(*draggedSlider_, mouse.x);
            }
        }
        if (const auto* movement = event.getIf<sf::Event::MouseMoved>(); movement && draggedSlider_)
        {
            const auto mouse = window.mapPixelToCoords(movement->position,
                                                       sf::View{sf::FloatRect{{0.0F, 0.0F}, vector(content_.referenceSize)}});
            updateSlider(*draggedSlider_, mouse.x);
        }
        const auto* release = event.getIf<sf::Event::MouseButtonReleased>();
        if (release == nullptr || release->button != sf::Mouse::Button::Left)
        {
            return std::nullopt;
        }
        mousePressed_ = false;
        draggedSlider_.reset();
        const auto mouse = window.mapPixelToCoords(release->position,
                                                   sf::View{sf::FloatRect{{0.0F, 0.0F}, vector(content_.referenceSize)}});
        if (settings_)
        {
            if (settingsBackBounds().contains(mouse)) settings_ = false;
            return std::nullopt;
        }
        if (buttonBounds(0).contains(mouse))
        {
            open_ = false;
            return PauseMenuAction::resume;
        }
        if (buttonBounds(1).contains(mouse)) return PauseMenuAction::saveAndMainMenu;
        if (context_ == PauseMenuContext::mainRoom && buttonBounds(2).contains(mouse))
        {
            settings_ = true;
            return std::nullopt;
        }
        if (context_ == PauseMenuContext::level && buttonBounds(2).contains(mouse)) return PauseMenuAction::abandonAndMainMenu;
        return std::nullopt;
    }

    sf::FloatRect PauseMenu::buttonBounds(std::size_t index) const
    {
        const auto position = vector(content_.pauseMenu.firstButton) +
                              sf::Vector2f{0.0F, static_cast<float>(index * content_.pauseMenu.buttonSpacing)};
        return {position, {static_cast<float>(button_.getSize().x / 3U), static_cast<float>(button_.getSize().y)}};
    }

    sf::FloatRect PauseMenu::settingsBackBounds() const
    {
        const auto position = vector(content_.pauseMenu.settingsBackButton);
        return {position, {static_cast<float>(button_.getSize().x / 3U), static_cast<float>(button_.getSize().y)}};
    }

    std::vector<std::string> PauseMenu::labels() const
    {
        if (context_ == PauseMenuContext::level)
        {
            return {"Weiterspielen", "Speichern und\nzum Hauptmenü", "Level aufgeben und\nzum Hauptraum"};
        }
        return {"Weiterspielen", "Speichern und\nzum Hauptmenü", "Einstellungen"};
    }

    void PauseMenu::drawButton(sf::RenderTarget& target, std::size_t index, std::string_view label, sf::Vector2f mouse) const
    {
        drawButton(target, buttonBounds(index), label, mouse);
    }

    void PauseMenu::drawButton(sf::RenderTarget& target, sf::FloatRect bounds, std::string_view label, sf::Vector2f mouse) const
    {
        const auto frameWidth = static_cast<int>(button_.getSize().x / 3U);
        const auto frame = bounds.contains(mouse) ? (mousePressed_ ? 2 : 1) : 0;
        sf::Sprite sprite{button_};
        sprite.setTextureRect({{frame * frameWidth, 0}, {frameWidth, static_cast<int>(button_.getSize().y)}});
        sprite.setPosition(bounds.position);
        target.draw(sprite);
        target.draw(centered(font_, sf::String::fromUtf8(label.begin(), label.end()),
                             static_cast<unsigned int>(content_.pauseMenu.textSize), bounds.getCenter()));
    }

    void PauseMenu::renderSettings(sf::RenderTarget& target, sf::Vector2f mouse) const
    {
        drawSlider(target, SettingsSlider::music);
        drawSlider(target, SettingsSlider::sound);
        drawSlider(target, SettingsSlider::battleSpeed);
        drawButton(target, settingsBackBounds(), "Zurück", mouse);
    }

    sf::FloatRect PauseMenu::sliderBounds(SettingsSlider slider) const
    {
        const auto position = vector(slider == SettingsSlider::music   ? content_.pauseMenu.musicSlider
                                     : slider == SettingsSlider::sound ? content_.pauseMenu.soundSlider
                                                                        : content_.pauseMenu.battleSpeedSlider);
        return {{position.x, position.y - 18.0F}, {static_cast<float>(content_.pauseMenu.sliderWidth), 36.0F}};
    }

    void PauseMenu::updateSlider(SettingsSlider slider, float mouseX)
    {
        const auto bounds = sliderBounds(slider);
        const auto ratio = std::clamp((mouseX - bounds.position.x) / bounds.size.x, 0.0F, 1.0F);
        if (slider == SettingsSlider::battleSpeed)
        {
            BattlePlaybackSettings::setSpeedPercent(static_cast<int>(std::round(50.0F + 150.0F * ratio)));
        }
        else if (slider == SettingsSlider::music)
        {
            AudioSettings::setMusicVolume(static_cast<int>(std::round(100.0F * ratio)));
        }
        else
        {
            AudioSettings::setSoundVolume(static_cast<int>(std::round(100.0F * ratio)));
        }
    }

    void PauseMenu::drawSlider(sf::RenderTarget& target, SettingsSlider slider) const
    {
        const auto bounds = sliderBounds(slider);
        const auto value = slider == SettingsSlider::music   ? AudioSettings::musicVolume()
                           : slider == SettingsSlider::sound ? AudioSettings::soundVolume()
                                                             : BattlePlaybackSettings::speedPercent();
        const auto label = slider == SettingsSlider::music   ? std::string{"Musik"}
                           : slider == SettingsSlider::sound ? std::string{"Ger\xC3\xA4usche"}
                                                             : std::string{"Kampfgeschwindigkeit"};
        target.draw(centered(font_, sf::String::fromUtf8(label.begin(), label.end()), 25U,
                             {bounds.getCenter().x, bounds.position.y - 32.0F}));
        sf::RectangleShape track{{bounds.size.x, 8.0F}};
        track.setPosition({bounds.position.x, bounds.getCenter().y - 4.0F});
        track.setFillColor(sf::Color{55, 31, 17});
        track.setOutlineColor(sf::Color{190, 126, 55});
        track.setOutlineThickness(2.0F);
        target.draw(track);
        sf::CircleShape knob{14.0F};
        knob.setOrigin({14.0F, 14.0F});
        const auto ratio = slider == SettingsSlider::battleSpeed ? static_cast<float>(value - 50) / 150.0F
                                                                  : static_cast<float>(value) / 100.0F;
        knob.setPosition({bounds.position.x + bounds.size.x * ratio, bounds.getCenter().y});
        knob.setFillColor(sf::Color{220, 151, 65});
        knob.setOutlineColor(sf::Color::Black);
        knob.setOutlineThickness(2.0F);
        target.draw(knob);
        target.draw(centered(font_, std::to_string(value) + " %", 20U,
                             {bounds.position.x + bounds.size.x + 55.0F, bounds.getCenter().y}));
    }

    void PauseMenu::render(sf::RenderTarget& target, sf::Vector2f mouse) const
    {
        if (!open_) return;
        if (auto* window = dynamic_cast<sf::Window*>(&target))
        {
            const auto interactive = settings_ ? sliderBounds(SettingsSlider::music).contains(mouse) ||
                                                     sliderBounds(SettingsSlider::sound).contains(mouse) ||
                                                     sliderBounds(SettingsSlider::battleSpeed).contains(mouse) ||
                                                     settingsBackBounds().contains(mouse)
                                               : buttonBounds(0).contains(mouse) || buttonBounds(1).contains(mouse) ||
                                                     buttonBounds(2).contains(mouse);
            setUiCursor(*window, interactive);
        }
        sf::RectangleShape shade{vector(content_.referenceSize)};
        shade.setFillColor(sf::Color{0, 0, 0, 150});
        target.draw(shade);
        sf::RectangleShape panel{vector(content_.pauseMenu.panelSize)};
        panel.setPosition(vector(content_.pauseMenu.panelPosition));
        panel.setFillColor(sf::Color{50, 27, 13, 245});
        panel.setOutlineColor(sf::Color{180, 116, 50});
        panel.setOutlineThickness(5.0F);
        target.draw(panel);
        target.draw(centered(font_, settings_ ? "Einstellungen" : "Pause",
                             static_cast<unsigned int>(content_.pauseMenu.titleSize), vector(content_.pauseMenu.titleCenter)));
        if (settings_)
        {
            renderSettings(target, mouse);
            return;
        }
        const auto entries = labels();
        for (std::size_t index = 0; index < entries.size(); ++index) drawButton(target, index, entries.at(index), mouse);
    }
} // namespace soma::presentation
