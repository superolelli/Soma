#pragma once

#include "presentation/video/IVideoPlayer.hpp"

namespace soma::presentation
{
    /// Adapts sfeMovie playback for boss introductions and other blocking game videos.
    class SfeMovieVideoPlayer final : public IVideoPlayer
    {
      public:
        [[nodiscard]] bool play(sf::RenderWindow& window, const std::filesystem::path& path) override;
    };
} // namespace soma::presentation
