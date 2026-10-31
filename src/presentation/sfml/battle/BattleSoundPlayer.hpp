#pragma once

#include "content/battle/BattleContent.hpp"
#include "presentation/battle/IBattleSoundPlayer.hpp"

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace soma::presentation
{
    /// Plays event-driven battle cues that are not embedded in Spriter animations.
    class BattleSoundPlayer final : public IBattleSoundPlayer
    {
      public:
        explicit BattleSoundPlayer(const content::BattleContent& content);

        void playActionStart(const std::vector<gameplay::GameplayEvent>& events) override;
        void playResults(const std::vector<gameplay::GameplayEvent>& events) override;

      private:
        void play(const std::string& cue);
        void removeStoppedSounds();

        std::unordered_map<std::string, sf::SoundBuffer> buffers_;
        std::vector<std::unique_ptr<sf::Sound>> activeSounds_;
    };
} // namespace soma::presentation
