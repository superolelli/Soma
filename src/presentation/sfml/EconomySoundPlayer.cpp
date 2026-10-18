#include "presentation/sfml/EconomySoundPlayer.hpp"

#include "presentation/sfml/AudioSettings.hpp"

#include <algorithm>

namespace soma::presentation
{
    EconomySoundPlayer::EconomySoundPlayer()
    {
        buffers_.emplace("click", sf::SoundBuffer{"Data/Sound/click.wav"});
        buffers_.emplace("inventory_drop", sf::SoundBuffer{"Data/Sound/inventory_drop.wav"});
        buffers_.emplace("loot_take_all", sf::SoundBuffer{"Data/Sound/lootable_take_all.wav"});
        buffers_.emplace("skill_acquired", sf::SoundBuffer{"Data/Sound/skill_acquired.wav"});
        buffers_.emplace("drink", sf::SoundBuffer{"Data/Sound/drink.wav"});
        buffers_.emplace("shop_upgrade", sf::SoundBuffer{"Data/Sound/Lootables/wagon.ogg"});
    }

    void EconomySoundPlayer::click()
    {
        play("click");
    }

    void EconomySoundPlayer::inventoryDrop()
    {
        play("inventory_drop");
    }

    void EconomySoundPlayer::lootTakeAll()
    {
        play("loot_take_all");
    }

    void EconomySoundPlayer::skillAcquired()
    {
        play("skill_acquired");
    }

    void EconomySoundPlayer::drink()
    {
        play("drink");
    }

    void EconomySoundPlayer::shopUpgrade()
    {
        play("shop_upgrade");
    }

    void EconomySoundPlayer::play(std::string_view cue)
    {
        std::erase_if(sounds_, [](const auto& sound) {
            return sound->getStatus() == sf::SoundSource::Status::Stopped;
        });
        auto sound = std::make_unique<sf::Sound>(buffers_.at(std::string{cue}));
        sound->setVolume(AudioSettings::sound());
        sound->play();
        sounds_.push_back(std::move(sound));
    }
} // namespace soma::presentation
