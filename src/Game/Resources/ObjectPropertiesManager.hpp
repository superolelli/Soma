#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include "../Util/XMLUtilities.hpp"

#include "../Ability.hpp"
#include "../LevelSpecs.hpp"

#include <map>


#define g_pObjectProperties ObjectPropertiesManager::Get()
class ObjectPropertiesManager : public TSingleton<ObjectPropertiesManager>
{
public:
	void LoadObjectProperties();

	Ability playerAbilities[4][4];
	CombatantStats playerStats[4];
	std::map<int, CombatantStats> enemyStats;

	Ability enemyAbilities[6];

	LevelSpecs levelSpecs;

private:
	void LoadPlayerAbilities();
	void LoadAbilitiesOfSpecificPlayer(const char* _path, int _id, Ability &_default);
	void LoadPlayerAttributes();
	void LoadEnemyAbilities();
	void LoadEnemyAttributes();
	void LoadLevelSpecs();
};

