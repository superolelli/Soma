#include "ObjectPropertiesManager.hpp"
#include "../CombatantID.hpp"
#include "StringManager.hpp"



std::unordered_map<std::string, ItemID> itemIdentifierMap;


void ObjectPropertiesManager::LoadObjectProperties()
{
	LoadEquipmentItemStats();
	LoadConsumableItemStats();
	LoadLootableProperties();
	LoadPlayerAbilities();
	LoadPlayerAttributes();
	LoadSkills();
	LoadEnemyAbilities();
	LoadEnemyAttributes();
	LoadLevelSpecs();
	LoadMainRoomPositions();
}


ItemProperties ObjectPropertiesManager::getItemStats(ItemID _id)
{
	if (_id < CONSUMABLE_ITEMS_START)
		return equipmentStats[_id];
	else
		return consumableStats[_id - CONSUMABLE_ITEMS_START];
}


void ObjectPropertiesManager::LoadPlayerAbilities()
{
    using namespace pugi;

    xml_document doc;
    doc.load_file("Data/XML/AbilitiesDefault.xml");

    //get default values
    Ability defaultAbility;
	defaultAbility.clear();

	loadAbilityFromXML(doc.child("ability"), defaultAbility);

	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesSimon.xml", CombatantID::Simon, defaultAbility);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesOle.xml", CombatantID::Ole, defaultAbility);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesAnna.xml", CombatantID::Anna, defaultAbility);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesMarkus.xml", CombatantID::Markus, defaultAbility);
}


void ObjectPropertiesManager::LoadAbilitiesOfSpecificPlayer(const char* _path, int _id, Ability &_default)
{
	using namespace pugi;

	xml_document doc;
	doc.load_file(_path);

	for (xml_node &ability : doc.child("Abilities").children())
	{
		int abilityID = ability.attribute("id").as_int();
		playerAbilities[_id][abilityID] = _default;
		loadAbilityFromXML(ability, playerAbilities[_id][abilityID]);
	}
}



void ObjectPropertiesManager::LoadPlayerAttributes()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/PlayerAttributes.xml");

	//load ability values
	for (xml_node &player : doc.child("PlayerAttributes").children())
	{
		int playerID = player.attribute("id").as_int();
		loadAttributesFromXML(player, playerStats[playerID]);
		std::string tempString = player.child("Name").text().as_string();
		combatantNames.push_back(sf::String::fromUtf8(tempString.begin(), tempString.end()));
	}
}


void ObjectPropertiesManager::LoadSkills()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/AbilitiesDefault.xml");

	//get default values
	Ability defaultAbility;
	defaultAbility.clear();

	loadAbilityFromXML(doc.child("ability"), defaultAbility);
	defaultAbility.possibleAims.howMany = 0;

	LoadSkillsOfSpecificPlayer("Data/XML/SkillsSimon.xml", CombatantID::Simon, defaultAbility);
	LoadSkillsOfSpecificPlayer("Data/XML/SkillsOle.xml", CombatantID::Ole, defaultAbility);
	LoadSkillsOfSpecificPlayer("Data/XML/SkillsAnna.xml", CombatantID::Anna, defaultAbility);
	LoadSkillsOfSpecificPlayer("Data/XML/SkillsMarkus.xml", CombatantID::Markus, defaultAbility);
}



void ObjectPropertiesManager::LoadSkillsOfSpecificPlayer(const char* _path, int _id, Ability &_default)
{
	using namespace pugi;

	xml_document doc;
	doc.load_file(_path);

	for (xml_node &ability : doc.child("Skills").children())
	{
		int abilityID = ability.attribute("abilityID").as_int();
		for (xml_node &skill : ability.children())
		{
			int skillID = skill.attribute("id").as_int();
			skills[_id][abilityID][skillID - 1] = _default;
			loadAbilityFromXML(skill, skills[_id][abilityID][skillID - 1]);
		}
	}
}

void ObjectPropertiesManager::LoadEnemyAbilities()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/AbilitiesDefault.xml");

	//get default values
	Ability defaultAbility;
	defaultAbility.clear();
	loadAbilityFromXML(doc.child("ability"), defaultAbility);

	doc.load_file("Data/XML/EnemyAbilities.xml");

	for (xml_node &ability : doc.child("Abilities").children())
	{
		int abilityID = ability.attribute("id").as_int();
		enemyAbilities[abilityID] = defaultAbility;

		loadAbilityFromXML(ability, enemyAbilities[abilityID]);
	}
}


void ObjectPropertiesManager::LoadEnemyAttributes()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/EnemyAttributes.xml");

	for (xml_node &enemy : doc.child("EnemyAttributes").children())
	{
		int enemyID = enemy.attribute("id").as_int();
		loadAttributesFromXML(enemy, enemyStats[enemyID]);
		loadPossibleLootFromXML(enemy, enemyLoot[enemyID]);
		std::string tempString = enemy.child("Name").text().as_string();
		combatantNames.push_back(sf::String::fromUtf8(tempString.begin(), tempString.end()));
	}
}


void ObjectPropertiesManager::LoadLevelSpecs()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/levelSpecsBang.xml");

	for (xml_node &level : doc.child("LevelSpecs").children())
	{
		int levelID = level.attribute("id").as_int();
		LevelSpecs specs;
		specs.levelType = LevelType::bang;
		loadLevelSpecsFromXML(level, specs);
		levelSpecs[LevelType::bang].push_back(specs);
	}

	doc.load_file("Data/XML/levelSpecsKutschfahrt.xml");
	for (xml_node& level : doc.child("LevelSpecs").children())
	{
		int levelID = level.attribute("id").as_int();
		LevelSpecs specs;
		specs.levelType = LevelType::kutschfahrt;
		loadLevelSpecsFromXML(level, specs);
		levelSpecs[LevelType::kutschfahrt].push_back(specs);
	}
}


void ObjectPropertiesManager::LoadMainRoomPositions()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/MainRoomPositions.xml");

	for (xml_node &door : doc.child("Positions").child("doors").children())
	{
		int doorID = door.attribute("id").as_int();
		mainRoomDoorPositions[doorID].x = door.attribute("x").as_int();
		mainRoomDoorPositions[doorID].y = door.attribute("y").as_int();
	}

	for (xml_node &sign : doc.child("Positions").child("signs").children())
	{
		int signID = sign.attribute("id").as_int();
		mainRoomSignPositions[signID].x = sign.attribute("x").as_int();
		mainRoomSignPositions[signID].y = sign.attribute("y").as_int();
	}

	for (xml_node &player : doc.child("Positions").child("players").children())
	{
		int playerID = player.attribute("id").as_int();
		mainRoomPlayerPositions[playerID].x = player.attribute("x").as_int();
		mainRoomPlayerPositions[playerID].y = player.attribute("y").as_int();
	}

	mainRoomRootsPosition.x = doc.child("Positions").child("roots").attribute("x").as_int();
	mainRoomRootsPosition.y = doc.child("Positions").child("roots").attribute("y").as_int();

	mainRoomVendingMachinePosition.x = doc.child("Positions").child("vendingMachine").attribute("x").as_int();
	mainRoomVendingMachinePosition.y = doc.child("Positions").child("vendingMachine").attribute("y").as_int();
}


void ObjectPropertiesManager::LoadLootableProperties()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/Lootables.xml");

	for (xml_node &lootable : doc.child("Lootables").children())
	{
		int lootableID = lootable.attribute("id").as_int();

		std::string name = lootable.attribute("name").as_string();
		lootableProperties[lootableID].name = sf::String::fromUtf8(name.begin(), name.end());

		lootableProperties[lootableID].boundingBox.left = lootable.child("boundingBox").child("left").text().as_int();
		lootableProperties[lootableID].boundingBox.top = lootable.child("boundingBox").child("top").text().as_int();
		lootableProperties[lootableID].boundingBox.width = lootable.child("boundingBox").child("width").text().as_int();
		lootableProperties[lootableID].boundingBox.height = lootable.child("boundingBox").child("height").text().as_int();

		lootableProperties[lootableID].position.x = lootable.child("xPosition").text().as_int();
		lootableProperties[lootableID].position.y = lootable.child("yPosition").text().as_int();

		if (lootable.child("loot"))
		{
			for (xml_node &item : lootable.child("loot").children())
			{
				lootableProperties[lootableID].possibleItems.push_back(itemIdentifierMap[item.text().as_string()]);
			}
		}

		int levelType = 0;
		if (lootable.child("levelType"))
			levelType = lootable.child("levelType").text().as_int();

		if (levelType == 0)
		{
			lootablesByLevelType[LevelType::bang].push_back(LootableID(lootableID));
			lootablesByLevelType[LevelType::kutschfahrt].push_back(LootableID(lootableID));
		}
		else
			lootablesByLevelType[levelType - 1].push_back(LootableID(lootableID));

	}
}


void ObjectPropertiesManager::LoadEquipmentItemStats()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/EquipmentItems.xml");

	//get default values
	EquipmentProperties defaulProperties;
	loadAttributesFromXML(doc.child("Items").child("Default"), defaulProperties.stats);
	defaulProperties.level = doc.child("Items").child("Default").child("Level").text().as_int();
	defaulProperties.name = "";
	defaulProperties.missOnHighDamage = doc.child("Items").child("Default").child("AdditionalEffects").child("MissOnHighDamage").text().as_bool();

	//load item values
	for (xml_node &item : doc.child("Items").children())
	{
		ItemID itemID = ItemID(item.attribute("id").as_int());

		if (itemID >= 0)
		{
			equipmentStats[itemID] = defaulProperties;
			int levelType = 0;

			std::string name = item.attribute("name").as_string();
			equipmentStats[itemID].name = sf::String::fromUtf8(name.begin(), name.end());

			itemIdentifierMap[name] = ItemID(itemID);

			loadAttributesFromXML(item, equipmentStats[itemID].stats);

			if(item.child("Level"))
				equipmentStats[itemID].level = item.child("Level").text().as_int();

			if (item.child("LevelType"))
				levelType = item.child("LevelType").text().as_int();

			if (item.child("AdditionalEffects"))
			{
				if (item.child("AdditionalEffects").child("MissOnHighDamage"))
					equipmentStats[itemID].missOnHighDamage = item.child("AdditionalEffects").child("MissOnHighDamage").text().as_bool();
			}

			equipmentStats[itemID].price = equipmentStats[itemID].level * EQUIPMENT_PRICE_PER_LEVEL;

			if (levelType == 0)
			{
				itemsByLevel[0][equipmentStats[itemID].level - 1].push_back(itemID);
				itemsByLevel[1][equipmentStats[itemID].level - 1].push_back(itemID);
			}
			else
				itemsByLevel[levelType - 1][equipmentStats[itemID].level-1].push_back(itemID);
		}
	}
}


void ObjectPropertiesManager::LoadConsumableItemStats()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/ConsumableItems.xml");

	//get default values
	ConsumableProperties defaulProperties;
	defaulProperties.heal = doc.child("Items").child("Default").child("effect").child("heal").text().as_int();
	defaulProperties.level = doc.child("Items").child("Default").child("Level").text().as_int();
	defaulProperties.name = "";
	defaulProperties.price = doc.child("Items").child("Default").child("Price").text().as_int();
	loadBuffFromXML(doc.child("Items").child("Default").child("effect").child("buff"), defaulProperties.buff);

	//load item values
	for (xml_node &item : doc.child("Items").children())
	{
		ItemID itemID = ItemID(item.attribute("id").as_int());

		if (itemID >= 0)
		{
			consumableStats[itemID] = defaulProperties;
			int levelType = 0;

			std::string name = item.attribute("name").as_string();
			consumableStats[itemID].name = sf::String::fromUtf8(name.begin(), name.end());

			itemIdentifierMap[name] = ItemID(itemID + CONSUMABLE_ITEMS_START);

			if(item.child("effect").child("heal"))
				consumableStats[itemID].heal = item.child("effect").child("heal").text().as_int();

			if (item.child("Level"))
				consumableStats[itemID].level = item.child("Level").text().as_int();

			if(item.child("Price"))
				consumableStats[itemID].price = item.child("Price").text().as_int();

			if (item.child("effect").child("buff"))
				loadBuffFromXML(item.child("effect").child("buff"), consumableStats[itemID].buff);

			if (item.child("LevelType"))
				levelType = item.child("LevelType").text().as_int();

			if (levelType == 0)
			{
				itemsByLevel[0][consumableStats[itemID].level - 1].push_back(ItemID(itemID + CONSUMABLE_ITEMS_START));
				itemsByLevel[1][consumableStats[itemID].level - 1].push_back(ItemID(itemID + CONSUMABLE_ITEMS_START));
			}
			else
				itemsByLevel[levelType - 1][consumableStats[itemID].level - 1].push_back(ItemID(itemID + CONSUMABLE_ITEMS_START));
		}
	}
}