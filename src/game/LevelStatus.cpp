#include "LevelStatus.hpp"
#include <functional>
#include "ObserverNotificationGameStatus.h"

LevelStatus::LevelStatus(int _difficulty)
    : difficulty(_difficulty)
    , opponentsKilled(0)
    , playersDied(0)
    , fatigue(0)
    , maxFatigue(MAX_FATIGUE_VALUE)
{
    for (auto& ability : abilityUsed)
        ability = false;
}


void LevelStatus::AddFatigue(int _fatigue)
{
    fatigue = std::min(fatigue + _fatigue, maxFatigue);
}

void LevelStatus::RemoveFatigue(int _fatigue)
{
    fatigue = std::max(0, fatigue - _fatigue);
}

void LevelStatus::OpponentKilled()
{
    opponentsKilled++;
}

void LevelStatus::PlayerDied()
{
    playersDied++;
}

void LevelStatus::SetAbilityUsed(int _player, int _ability)
{
    abilityUsed[_player * 4 + _ability] = true;
}


bool LevelStatus::AllAbilitiesUsed() const
{
    return std::all_of(abilityUsed.begin(), abilityUsed.end(), [](bool used) {return used; });
}



float LevelStatus::GetRelativeFatigue() const
{
    return static_cast<float>(fatigue) / static_cast<float>(maxFatigue);
}

void LevelStatus::OnNotify(ObserverNotification const& _notification)
{
    auto notification = dynamic_cast<ObserverNotificationGameStatus const*>(&_notification);

    if (notification->event == gameStatusEvents::itemUnlocked)
    {
        unlockedItems.insert(notification->item.id);
    }
}