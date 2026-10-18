#include "presentation/sfml/BattleSoundPlayer.hpp"

#include "presentation/sfml/AudioSettings.hpp"

#include <algorithm>

namespace soma::presentation
{
    BattleSoundPlayer::BattleSoundPlayer(const content::BattleContent& content)
    {
        for (const auto& [cue, path] : content.sounds)
        {
            buffers_.emplace(cue, sf::SoundBuffer{path});
        }
    }

    void BattleSoundPlayer::playActionStart(const std::vector<gameplay::GameplayEvent>& events)
    {
        removeStoppedSounds();
        if (std::ranges::any_of(events, [](const auto& event) {
                return std::holds_alternative<gameplay::AttackDodged>(event);
            }))
        {
            play("attack_dodged");
        }
    }

    void BattleSoundPlayer::playResults(const std::vector<gameplay::GameplayEvent>& events)
    {
        removeStoppedSounds();
        for (const auto& event : events)
        {
            if (std::holds_alternative<gameplay::CriticalHit>(event))
            {
                play("critical_hit");
            }
            else if (const auto* triggered = std::get_if<gameplay::StatusTriggered>(&event))
            {
                play(triggered->status.value());
            }
        }
    }

    void BattleSoundPlayer::play(const std::string& cue)
    {
        const auto found = buffers_.find(cue);
        if (found == buffers_.end())
        {
            return;
        }
        auto sound = std::make_unique<sf::Sound>(found->second);
        sound->setVolume(AudioSettings::sound());
        sound->play();
        activeSounds_.push_back(std::move(sound));
    }

    void BattleSoundPlayer::removeStoppedSounds()
    {
        std::erase_if(activeSounds_, [](const auto& sound) {
            return sound->getStatus() == sf::SoundSource::Status::Stopped;
        });
    }
} // namespace soma::presentation
