#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include "../Util/XMLUtilities.hpp"
#include <SFML\Graphics.hpp>

#include "../Ability.hpp"
#include "../LevelSpecs.hpp"

#include "../Item.hpp"

#include <map>

struct LootableProperties
{
	sf::String name;
	sf::IntRect boundingBox;
	sf::Vector2i position;
	std::vector<ItemID> possibleItems;
};

struct ItemProperties
{
	CombatantStats stats;
	sf::String name;
	int level;
};

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

	LootableProperties lootableProperties[11];

	ItemProperties itemStats[4];

	std::vector<ItemID> itemsByLevel[10];

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
	void LoadLootableProperties();
	void LoadItemStats();
};

