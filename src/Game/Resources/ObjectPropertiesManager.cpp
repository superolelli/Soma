#include "ObjectPropertiesManager.hpp"
#include "../CombatantID.hpp"




void ObjectPropertiesManager::LoadObjectProperties()
{
	LoadPlayerAbilities();
	LoadPlayerAttributes();
	LoadSkills();
	LoadEnemyAbilities();
	LoadEnemyAttributes();
	LoadLevelSpecs();
	LoadMainRoomPositions();
	LoadItemStats();
	LoadLootableProperties();
}




void ObjectPropertiesManager::LoadPlayerAbilities()
{
    using namespace pugi;

    xml_document doc;
    doc.load_file("Data/XML/AbilitiesDefault.xml");

    //get default values
    Ability default;
	default.clear();

	loadAbilityFromXML(doc.child("ability"), default);

	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesSimon.xml", CombatantID::Simon, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesOle.xml", CombatantID::Ole, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesAnna.xml", CombatantID::Anna, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesMarkus.xml", CombatantID::Markus, default);
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
	}
}


void ObjectPropertiesManager::LoadSkills()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/AbilitiesDefault.xml");

	//get default values
	Ability default;
	default.clear();

	loadAbilityFromXML(doc.child("ability"), default);
	default.possibleAims.howMany = 0;

	LoadSkillsOfSpecificPlayer("Data/XML/SkillsSimon.xml", CombatantID::Simon, default);
	LoadSkillsOfSpecificPlayer("Data/XML/SkillsOle.xml", CombatantID::Ole, default);
	LoadSkillsOfSpecificPlayer("Data/XML/SkillsAnna.xml", CombatantID::Anna, default);
	LoadSkillsOfSpecificPlayer("Data/XML/SkillsMarkus.xml", CombatantID::Markus, default);
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
	Ability default;
	default.clear();
	loadAbilityFromXML(doc.child("ability"), default);

	doc.load_file("Data/XML/EnemyAbilities.xml");

	for (xml_node &ability : doc.child("Abilities").children())
	{
		int abilityID = ability.attribute("id").as_int();
		enemyAbilities[abilityID] = default;

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
	}
}


void ObjectPropertiesManager::LoadLevelSpecs()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/LevelSpecs.xml");

	for (xml_node &level : doc.child("LevelSpecs").children())
	{
		int levelID = level.attribute("id").as_int();
		loadLevelSpecsFromXML(level, levelSpecs[levelID - 1]);
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
	}
}


void ObjectPropertiesManager::LoadItemStats()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/Items.xml");

	//get default values
	ItemProperties default;
	loadAttributesFromXML(doc.child("Items").child("Default"), default.stats);
	default.level = doc.child("Items").child("Default").child("Level").text().as_int();
	default.name = "";

	//load item values
	for (xml_node &item : doc.child("Items").children())
	{
		ItemID itemID = ItemID(item.attribute("id").as_int());

		if (itemID >= 0)
		{
			itemStats[itemID] = default;

			std::string name = item.attribute("name").as_string();
			itemStats[itemID].name = sf::String::fromUtf8(name.begin(), name.end());

			itemIdentifierMap[name] = ItemID(itemID);

			loadAttributesFromXML(item, itemStats[itemID].stats);

			if(item.child("Level"))
				itemStats[itemID].level = item.child("Level").text().as_int();

			itemsByLevel[itemStats[itemID].level-1].push_back(itemID);
		}
	}
}