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