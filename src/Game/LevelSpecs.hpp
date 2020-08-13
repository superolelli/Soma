#pragma once

#include <vector>
#include "CombatantID.hpp"
#include "Resources\BackgroundIDs.hpp"
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
	BackgroundID endBackground;
	std::vector<BackgroundID> possibleBackgrounds;
	LevelReward reward;
};