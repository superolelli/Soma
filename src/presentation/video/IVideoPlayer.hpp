#pragma once

#include <filesystem>

namespace sf
{
    class RenderWindow;
}

namespace soma::presentation
{
    /// Decouples battle flow from the concrete library used to play boss videos.
    class IVideoPlayer
    {
      public:
        virtual ~IVideoPlayer() = default;
        [[nodiscard]] virtual bool play(sf::RenderWindow& window, const std::filesystem::path& path) = 0;
    };
} // namespace soma::presentation
