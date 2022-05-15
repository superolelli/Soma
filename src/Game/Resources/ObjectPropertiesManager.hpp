#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include "../Util/XMLUtilities.hpp"
#include <SFML\Graphics.hpp>

#include "../Ability.hpp"
#include "../LevelSpecs.hpp"
#include "../Lootable.hpp"

#include "../Item.hpp"

#include "EnemyAbilities.hpp"

#include <map>

struct LootableProperties
{
	sf::String name;
	sf::IntRect boundingBox;
	sf::Vector2i position;
	std::vector<ItemID> possibleItems;
};

#define g_pObjectProperties ObjectPropertiesManager::Get()
class ObjectPropertiesManager : public TSingleton<ObjectPropertiesManager>
{
public:
	void LoadObjectProperties();

	Ability playerAbilities[4][4];
	CombatantAttributes playerStats[4];
	std::map<int, CombatantAttributes> enemyStats;
	std::unordered_map<int, std::vector<ItemID>> enemyLoot;
	std::vector<sf::String> combatantNames;

	Ability enemyAbilities[int(enemyAbilities::number_of_abilities)];

	Ability skills[4][4][8];

	std::vector<LevelSpecs> levelSpecs[2];

	LootableProperties lootableProperties[LootableID::numberOfLootables];
	std::vector<LootableID> lootablesByLevelType[2];

	EquipmentProperties equipmentStats[NUMBER_OF_EQUIPMENT_ITEMS];
	ConsumableProperties consumableStats[NUMBER_OF_CONSUMABLE_ITEMS];

	std::vector<ItemID> itemsByLevel[2][10];

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

