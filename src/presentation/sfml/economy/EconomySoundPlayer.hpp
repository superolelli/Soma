#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace soma::presentation
{
    /// Owns reusable one-shot UI and economy sounds so short cues survive individual widgets.
    class EconomySoundPlayer
    {
      public:
        EconomySoundPlayer();

        void click();
        void inventoryDrop();
        void lootTakeAll();
        void skillAcquired();
        void drink();
        void shopUpgrade();

      private:
        void play(std::string_view cue);

        std::unordered_map<std::string, sf::SoundBuffer> buffers_;
        std::vector<std::unique_ptr<sf::Sound>> sounds_;
    };
} // namespace soma::presentation
