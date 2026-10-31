#pragma once

#include "gameplay/random/IRandom.hpp"

#include <SFML/Audio.hpp>
#include <filesystem>
#include <optional>
#include <vector>

namespace soma::presentation
{
    /// Sequences exploration tracks and crossfades cleanly into and out of battle music.
    class ExplorationMusicPlayer
    {
      public:
        ExplorationMusicPlayer(std::vector<std::filesystem::path> tracks,
                               gameplay::IRandom& random,
                               float musicVolume,
                               float battleVolume,
                               float crossfadeSeconds);

        void update();
        void pauseForBattle(const std::filesystem::path& battleTrack);
        void startBattleMusic();
        void resumeAfterBattle();

      private:
        void playNext();

        std::vector<std::filesystem::path> tracks_;
        std::vector<std::size_t> remainingTracks_;
        gameplay::IRandom& random_;
        std::optional<sf::Music> current_;
        std::optional<sf::Music> battle_;
        sf::Clock updateClock_;
        float musicVolume_{};
        float battleVolume_{};
        float crossfadeSeconds_{};
        float crossfadeRemaining_{};
        bool pausedForBattle_{};
    };
} // namespace soma::presentation
