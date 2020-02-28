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
	sf::String name;
	int level;
};

struct EquipmentProperties : public ItemProperties
{
	CombatantStats stats;
};

struct ConsumableProperties : public ItemProperties
{
	int health;
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

	EquipmentProperties equipmentStats[NUMBER_OF_EQUIPMENT_ITEMS];
	ConsumableProperties consumableStats[NUMBER_OF_CONSUMABLE_ITEMS];

	std::vector<ItemID> itemsByLevel[10];

	ItemProperties getItemStats(ItemID _id);

	sf::Vector2i mainRoomDoorPositions[3];
	sf::Vector2i mainRoomPlayerPositions[4];
	sf::Vector2i mainRoomSignPositions[3];
	sf::Vector2i mainRoomRootsPosition;
	sf::Vector2i mainRoomVendingMachinePosition;

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
	void LoadEquipmentItemStats();
	void LoadConsumableItemStats();
};

