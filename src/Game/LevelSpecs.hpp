#pragma once

#include <vector>
#include "CombatantID.hpp"
#include <array>

struct LevelReward {
	int dice;
	int cards;
};

enum LevelType {bang, kutschfahrt, tichu};

struct LevelSpecs {
	LevelType levelType;
	int level;
	float battleProbability;
	float lootableProbability;
	int numberOfRooms;
	std::vector<std::array<CombatantID, 4>> possibleEnemyGroups;
	std::array<CombatantID, 4> bossGroup;
	int endBackground;
	std::vector<int> possibleBackgrounds;
	LevelReward reward;
};