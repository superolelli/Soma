#pragma once

#include "content/GameFlowContent.hpp"
#include "presentation/sfml/SpriterAnimationPlayer.hpp"

#include <SFML/Graphics.hpp>

namespace soma::presentation
{
    /// Keeps an animated loading/error screen responsive while the application initializes content.
    class StartupScreen
    {
      public:
        StartupScreen(sf::RenderWindow& window, const content::GameFlowContent& content);

        [[nodiscard]] bool processEvents();
        void renderLoading();
        void showError(std::string_view message);

      private:
        [[nodiscard]] sf::Text wrappedError(std::string_view message) const;

        sf::RenderWindow& window_;
        const content::GameFlowContent& content_;
        SpriterAnimationPlayer animation_;
        sf::Font font_;
        sf::Clock clock_;
    };
} // namespace soma::presentation
