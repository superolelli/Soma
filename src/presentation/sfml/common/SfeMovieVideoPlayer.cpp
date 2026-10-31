#include "presentation/sfml/common/SfeMovieVideoPlayer.hpp"

#include <SFML/Graphics.hpp>
#include <sfeMovie/Movie.hpp>

namespace soma::presentation
{
    bool SfeMovieVideoPlayer::play(sf::RenderWindow& window, const std::filesystem::path& path)
    {
        sfe::Movie movie;
        if (!movie.openFromFile(path.string()))
        {
            return false;
        }
        const auto movieSize = movie.getSize();
        const auto viewSize = window.getView().getSize();
        movie.setScale({viewSize.x / movieSize.x, viewSize.y / movieSize.y});
        movie.setPosition(window.getView().getCenter() - viewSize / 2.0F);
        movie.play();
        while (window.isOpen() && movie.getStatus() == sfe::Status::Playing)
        {
            while (const auto event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                }
                if (const auto* key = event->getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape)
                {
                    movie.stop();
                }
            }
            movie.update();
            window.clear(sf::Color::Black);
            window.draw(movie);
            window.display();
        }

        // Ensure sfeMovie's audio stream is stopped before battle audio starts.
        // In particular, this also covers natural end-of-file and window-close.
        if (movie.getStatus() != sfe::Status::Stopped)
        {
            movie.stop();
        }
        return window.isOpen();
    }
} // namespace soma::presentation
