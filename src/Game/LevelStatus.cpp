#include "LevelStatus.hpp"
#include <functional>

void LevelStatus::Reset(int _difficulty)
{
    difficulty = _difficulty;
    opponentsKilled = 0;
    playersDied = 0;
    ResetFatigue();
}


void LevelStatus::AddFatigue(int _fatigue)
{
    fatigue = std::min(fatigue + _fatigue, maxFatigue);
}

void LevelStatus::RemoveFatigue(int _fatigue)
{
    fatigue = std::max(0, fatigue - _fatigue);
}

void LevelStatus::ResetFatigue()
{
    maxFatigue = MAX_FATIGUE_VALUE;
    fatigue = 0;
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