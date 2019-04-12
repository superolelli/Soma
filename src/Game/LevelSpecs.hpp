#pragma once

#include <vector>
#include "CombatantID.hpp"
#include "Resources\BackgroundIDs.hpp"
#include <array>

struct LevelSpecs {
	int level;
	float battleProbability;
	int numberOfRooms;
	std::vector<std::array<CombatantID, 4>> possibleEnemyGroups;
	BackgroundID endBackground;
	std::vector<BackgroundID> possibleBackgrounds;
};