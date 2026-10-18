#pragma once

#include "content/GameFlowContent.hpp"

#include <SFML/Graphics.hpp>
#include <optional>

namespace soma::presentation
{
    enum class PauseMenuContext
    {
        mainRoom,
        level
    };

    enum class PauseMenuAction
    {
        resume,
        saveAndMainMenu,
        abandonAndMainMenu
    };

    enum class SettingsSlider
    {
        music,
        sound,
        battleSpeed
    };

    /// Provides context-sensitive pause actions and persistent audio/playback settings.
    class PauseMenu
    {
      public:
        PauseMenu(const content::GameFlowContent& content, PauseMenuContext context);

        void open();
        [[nodiscard]] bool isOpen() const noexcept;
        [[nodiscard]] std::optional<PauseMenuAction> handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void render(sf::RenderTarget& target, sf::Vector2f mouse) const;

      private:
        [[nodiscard]] sf::FloatRect buttonBounds(std::size_t index) const;
        [[nodiscard]] sf::FloatRect settingsBackBounds() const;
        [[nodiscard]] std::vector<std::string> labels() const;
        void drawButton(sf::RenderTarget& target, std::size_t index, std::string_view label, sf::Vector2f mouse) const;
        void drawButton(sf::RenderTarget& target, sf::FloatRect bounds, std::string_view label, sf::Vector2f mouse) const;
        void renderSettings(sf::RenderTarget& target, sf::Vector2f mouse) const;
        [[nodiscard]] sf::FloatRect sliderBounds(SettingsSlider slider) const;
        void updateSlider(SettingsSlider slider, float mouseX);
        void drawSlider(sf::RenderTarget& target, SettingsSlider slider) const;

        const content::GameFlowContent& content_;
        PauseMenuContext context_{};
        sf::Texture button_;
        sf::Font font_;
        bool open_{};
        bool settings_{};
        bool mousePressed_{};
        std::optional<SettingsSlider> draggedSlider_;
    };
} // namespace soma::presentation
