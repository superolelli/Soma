#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include "../Util/XMLUtilities.hpp"
#include <SFML\Graphics.hpp>

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

	Ability enemyAbilities[10];

	Ability skills[4][4][8];

	LevelSpecs levelSpecs[5];

	sf::IntRect lootablesBoundingBoxes[11];
	sf::Vector2i lootablePositions[11];

	std::pair<std::string, CombatantStats> itemStats[2];

	sf::Vector2i mainRoomDoorPositions[3];
	sf::Vector2i mainRoomPlayerPositions[4];
	sf::Vector2i mainRoomSignPositions[3];
	sf::Vector2i mainRoomRootsPosition;

private:
	void LoadPlayerAbilities();
	void LoadAbilitiesOfSpecificPlayer(const char* _path, int _id, Ability &_default);
	void LoadPlayerAttributes();

	void LoadSkills();
	void LoadSkillsOfSpecificPlayer(const char* _path, int _id, Ability &_default);

	void LoadEnemyAbilities();
	void LoadEnemyAttributes();
	void LoadLevelSpecs();
	void LoadMainRoomPositions();
	void LoadLootablesBoundingBoxes();
	void LoadItemStats();
};

