#include "ObjectPropertiesManager.hpp"
#include "../PlayerEnum.hpp"




void ObjectPropertiesManager::LoadObjectProperties()
{
	LoadPlayerAbilities();
	LoadPlayerAttributes();
	LoadEnemyAbilities();
	LoadEnemyAttributes();
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

	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesSimon.xml", PlayerID::Simon, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesOle.xml", PlayerID::Ole, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesAnna.xml", PlayerID::Anna, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesMarkus.xml", PlayerID::Markus, default);
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



void ObjectPropertiesManager::LoadEnemyAbilities()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/AbilitiesDefault.xml");

	//get default values
	Ability default;
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

	for (xml_node enemy : doc.child("EnemyAttributes").children())
	{
		int enemyID = enemy.attribute("id").as_int();

		loadAttributesFromXML(enemy, enemyStats[enemyID]);
	}
}