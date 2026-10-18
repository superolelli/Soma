#include "presentation/sfml/ExplorationMusicPlayer.hpp"

#include "presentation/sfml/AudioSettings.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace soma::presentation
{
    ExplorationMusicPlayer::ExplorationMusicPlayer(std::vector<std::filesystem::path> tracks,
                                                   gameplay::IRandom& random,
                                                   float musicVolume,
                                                   float battleVolume,
                                                   float crossfadeSeconds)
        : tracks_(std::move(tracks))
        , random_(random)
        , musicVolume_(musicVolume)
        , battleVolume_(battleVolume)
        , crossfadeSeconds_(crossfadeSeconds)
    {
        if (tracks_.empty())
        {
            throw std::invalid_argument("Exploration music requires at least one track");
        }
        if (musicVolume_ < 0.0F || battleVolume_ < 0.0F || crossfadeSeconds_ <= 0.0F)
        {
            throw std::invalid_argument("Exploration music volumes and crossfade duration must be valid");
        }
        playNext();
    }

    void ExplorationMusicPlayer::update()
    {
        const auto elapsed = updateClock_.restart().asSeconds();
        if (crossfadeRemaining_ > 0.0F && current_ && battle_)
        {
            crossfadeRemaining_ = std::max(0.0F, crossfadeRemaining_ - elapsed);
            const auto battleFraction = crossfadeRemaining_ / crossfadeSeconds_;
            current_->setVolume(AudioSettings::music(musicVolume_ * (1.0F - battleFraction)));
            battle_->setVolume(AudioSettings::music(musicVolume_ * battleFraction));
            if (crossfadeRemaining_ <= 0.0F)
            {
                battle_->stop();
                battle_.reset();
            }
        }
        if (!pausedForBattle_ && current_ && current_->getStatus() == sf::SoundSource::Status::Stopped)
        {
            playNext();
        }
        else if (!pausedForBattle_ && current_ && crossfadeRemaining_ <= 0.0F)
        {
            current_->setVolume(AudioSettings::music(musicVolume_));
        }
    }

    void ExplorationMusicPlayer::pauseForBattle(const std::filesystem::path& battleTrack)
    {
        pausedForBattle_ = true;
        if (current_)
        {
            current_->pause();
        }
        battle_.emplace(battleTrack);
        battle_->setLooping(true);
        battle_->setRelativeToListener(true);
        battle_->setVolume(AudioSettings::music(battleVolume_));
    }

    void ExplorationMusicPlayer::startBattleMusic()
    {
        if (!pausedForBattle_ || !battle_)
        {
            return;
        }
        battle_->play();
    }

    void ExplorationMusicPlayer::resumeAfterBattle()
    {
        pausedForBattle_ = false;
        if (current_)
        {
            current_->setVolume(0.0F);
            current_->play();
        }
        crossfadeRemaining_ = crossfadeSeconds_;
        updateClock_.restart();
    }

    void ExplorationMusicPlayer::playNext()
    {
        if (remainingTracks_.empty())
        {
            remainingTracks_.resize(tracks_.size());
            std::iota(remainingTracks_.begin(), remainingTracks_.end(), 0U);
        }
        const auto position = static_cast<std::size_t>(random_.between(0, static_cast<int>(remainingTracks_.size()) - 1));
        const auto track = remainingTracks_.at(position);
        remainingTracks_.erase(remainingTracks_.begin() + static_cast<std::ptrdiff_t>(position));
        current_.emplace(tracks_.at(track));
        current_->setRelativeToListener(true);
        current_->setVolume(AudioSettings::music(musicVolume_));
        current_->play();
    }
} // namespace soma::presentation
