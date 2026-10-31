#include "presentation/sfml/app/StartupScreen.hpp"

#include <ranges>

namespace soma::presentation
{
    StartupScreen::StartupScreen(sf::RenderWindow& window, const content::GameFlowContent& content)
        : window_(window)
        , content_(content)
        , animation_(window, content.startup.model, content.startup.entity)
        , font_(content.assets.at("font"))
    {
        animation_.setPosition(static_cast<float>(content.startup.position.x), static_cast<float>(content.startup.position.y));
        animation_.play(content.startup.animation, static_cast<float>(content.startup.speed));
    }

    bool StartupScreen::processEvents()
    {
        while (const auto event = window_.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window_.close();
                return false;
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape)
            {
                window_.close();
                return false;
            }
        }
        return window_.isOpen();
    }

    void StartupScreen::renderLoading()
    {
        animation_.update(static_cast<float>(clock_.restart().asMilliseconds()));
        window_.clear(sf::Color::Black);
        animation_.render();
        window_.display();
    }

    void StartupScreen::showError(std::string_view message)
    {
        while (processEvents())
        {
            window_.clear(sf::Color{25, 16, 18});
            sf::Text title{font_, "Soma konnte nicht gestartet werden", 48U};
            title.setPosition({180.0F, 220.0F});
            title.setFillColor(sf::Color{235, 185, 95});
            window_.draw(title);
            window_.draw(wrappedError(message));
            sf::Text hint{font_, "Escape zum Beenden", 26U};
            hint.setPosition({180.0F, 780.0F});
            window_.draw(hint);
            window_.display();
        }
    }

    sf::Text StartupScreen::wrappedError(std::string_view message) const
    {
        constexpr std::size_t charactersPerLine = 78;
        std::string wrapped;
        std::size_t lineLength{};
        for (const auto word : std::views::split(message, ' '))
        {
            const std::string value{word.begin(), word.end()};
            if (lineLength != 0 && lineLength + value.size() + 1 > charactersPerLine)
            {
                wrapped += '\n';
                lineLength = 0;
            }
            if (lineLength != 0)
            {
                wrapped += ' ';
                ++lineLength;
            }
            wrapped += value;
            lineLength += value.size();
        }
        sf::Text text{font_, sf::String::fromUtf8(wrapped.begin(), wrapped.end()), 30U};
        text.setPosition({180.0F, 340.0F});
        text.setFillColor(sf::Color::White);
        return text;
    }
} // namespace soma::presentation
