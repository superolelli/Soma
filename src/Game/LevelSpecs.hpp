#pragma once

#include <vector>
#include "CombatantID.hpp"
#include "Resources\BackgroundIDs.hpp"
#include <array>

struct LevelReward {
	int dice;
	int cards;
};

struct LevelSpecs {
	int level;
	float battleProbability;
	int numberOfRooms;
	std::vector<std::array<CombatantID, 4>> possibleEnemyGroups;
	std::array<CombatantID, 4> bossGroup;
	BackgroundID endBackground;
	std::vector<BackgroundID> possibleBackgrounds;
	LevelReward reward;
};