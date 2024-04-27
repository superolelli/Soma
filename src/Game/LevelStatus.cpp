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