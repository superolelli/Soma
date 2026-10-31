#pragma once

#include "content/flow/GameFlowContent.hpp"
#include "persistence/SaveSlotService.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>

namespace soma::presentation
{
    struct MainMenuSelection
    {
        int slot{};
        persistence::SaveGame saveGame;
    };

    /// Manages the three save slots, player-name entry, and transition into a game session.
    class MainMenuScreen
    {
      public:
        MainMenuScreen(sf::RenderWindow& window,
                       const content::GameFlowContent& content,
                       const persistence::SaveSlotService& slots);

        [[nodiscard]] std::optional<MainMenuSelection> run();

      private:
        void processEvent(const sf::Event& event);
        void handleClick(sf::Vector2f position);
        void handleNameInput(char32_t character);
        void submitName();
        void refreshSlots();
        void render();
        void drawSlot(const persistence::SaveSlotSummary& slot) const;
        void drawButton(sf::Vector2f position, std::string_view label, sf::Vector2f mouse) const;
        [[nodiscard]] sf::FloatRect buttonBounds(sf::Vector2f position) const;
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;

        sf::RenderWindow& window_;
        const content::GameFlowContent& content_;
        const persistence::SaveSlotService& slots_;
        sf::Texture background_;
        sf::Texture savePanel_;
        sf::Texture nameInputPanel_;
        sf::Texture button_;
        sf::Texture closeButton_;
        sf::Font font_;
        std::optional<sf::Music> music_;
        std::array<persistence::SaveSlotSummary, persistence::SaveSlotService::slotCount> slotSummaries_;
        std::optional<int> activeSlot_;
        std::optional<MainMenuSelection> selection_;
        sf::String enteredName_;
        std::string feedback_;
        bool enteringName_{};
        bool mousePressed_{};
    };
} // namespace soma::presentation
