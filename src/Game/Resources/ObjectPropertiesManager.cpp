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
    PlayerAbility default;
	loadAbilityFromXML(doc.child("ability"), default);

	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesSimon.xml", PlayerID::Simon, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesOle.xml", PlayerID::Ole, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesAnna.xml", PlayerID::Anna, default);
	LoadAbilitiesOfSpecificPlayer("Data/XML/AbilitiesMarkus.xml", PlayerID::Markus, default);
}


void ObjectPropertiesManager::LoadAbilitiesOfSpecificPlayer(const char* _path, int _id, PlayerAbility &_default)
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
	doc.load_file("Data/XML/EnemyAbilities.xml");

	//get default values
	Ability default;
	default.possibleAims.howMany = doc.child("EnemyAbilities").child("default").attribute("how_many").as_int();
	default.name = doc.child("EnemyAbilities").child("default").attribute("name").as_string();
	for (xml_node pos : doc.child("EnemyAbilities").child("default").child("positions").children())
	{
		default.possibleAims.position[pos.attribute("id").as_int()] = pos.text().as_bool();
	}

	//load ability values
	for (xml_node ability : doc.child("EnemyAbilities").children())
	{
		int abilityID = ability.attribute("id").as_int();

		if (abilityID >= 0)
		{
			enemyAbilities[abilityID] = default;

			if (ability.attribute("how_many"))
				enemyAbilities[abilityID].possibleAims.howMany = ability.attribute("how_many").as_int();

			enemyAbilities[abilityID].name = ability.attribute("name").as_string();

			for (xml_node pos : ability.child("positions").children())
			{
				enemyAbilities[abilityID].possibleAims.position[pos.attribute("id").as_int()] = pos.text().as_bool();
			}
		}
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