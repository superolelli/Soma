#pragma once

#include <vector>
#include <string>
#include "LevelStatus.hpp"

struct LevelRewards {
    std::vector<std::tuple<int, int, std::string>> achievements;
    std::vector<int> unlockedItems;
};


static void GetLevelRewards(LevelRewards& rewards, const LevelStatus& _status, bool _levelFinished)
{
    // TODO: Unlocked items

    if (_levelFinished)
    {
        rewards.achievements.emplace_back(10 * _status.difficulty, 1 * _status.difficulty, "Level geschafft");
    }

    if (_status.playersDied == 0)
    {
        rewards.achievements.emplace_back(20, 0, "Niemanden zurückgelassen");
    }

    if (_status.playersDied == 3)
    {
        rewards.achievements.emplace_back(30, 0, "Knappe Kiste");
    }

    if (_status.opponentsKilled > 0)
    {
        rewards.achievements.emplace_back(2 * _status.opponentsKilled, 0, std::to_string(_status.opponentsKilled) + " besiegte Gegner");
    }

    if (_status.GetRelativeFatigue() > 0.66)
    {
        rewards.achievements.emplace_back(30, 0, "Der Müdigkeit zum Trotz");
    }
}