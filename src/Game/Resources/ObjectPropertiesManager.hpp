#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include "../Util/XMLUtilities.hpp"

#include "../Ability.hpp"

#include <map>


struct Ability
{
	PossibleAims possibleAims;
	std::string name;
};


#define g_pObjectProperties ObjectPropertiesManager::Get()
class ObjectPropertiesManager : public TSingleton<ObjectPropertiesManager>
{
public:
	void LoadObjectProperties();

	PlayerAbility playerAbilities[4][4];
	CombatantAttributes playerAttributes[4];
	std::map<int, CombatantAttributes> enemyAttributes;

	Ability enemyAbilities[2];

private:
	void LoadPlayerAbilities();
	void LoadPlayerAttributes();
	void LoadEnemyAbilities();
	void LoadEnemyAttributes();
};

