#include "ObjectPropertiesManager.hpp"




void ObjectPropertiesManager::LoadObjectProperties()
{
	LoadPlayerAbilities();
	LoadPlayerAttributes();
	LoadEnemyAbilities();
}



void ObjectPropertiesManager::LoadPlayerAbilities()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/PlayerAbilities.xml");

	//get default values
	PossibleAims default;
	default.howMany = doc.child("PlayerAbilities").child("default").attribute("how_many").as_int();
	for (xml_node pos : doc.child("PlayerAbilities").child("default").child("positions").children())
	{
		default.position[pos.attribute("id").as_int()] = pos.text().as_bool();
	}

	//load ability values
	for (xml_node player : doc.child("PlayerAbilities").children())
	{
		int playerID = player.attribute("id").as_int();
		if (playerID >= 0)
		{
			for (xml_node ability : player.children())
			{
				int abilityID = ability.attribute("id").as_int();
				playerAbilities[playerID][abilityID] = default;

				if (ability.attribute("how_many"))
					playerAbilities[playerID][abilityID].howMany = ability.attribute("how_many").as_int();

				for (xml_node pos : ability.child("positions").children())
				{
					playerAbilities[playerID][abilityID].position[pos.attribute("id").as_int()] = pos.text().as_bool();
				}
			}
		}
	}
}




void ObjectPropertiesManager::LoadPlayerAttributes()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/PlayerAttributes.xml");

	//load ability values
	for (xml_node player : doc.child("PlayerAttributes").children())
	{
		int playerID = player.attribute("id").as_int();
		
		playerAttributes[playerID].armour = player.attribute("armour").as_int();
		playerAttributes[playerID].maxHealth= player.attribute("health").as_int();
		playerAttributes[playerID].damage = player.attribute("damage").as_int();
		playerAttributes[playerID].initiative = player.attribute("initiative").as_int();
		playerAttributes[playerID].dexterity = player.attribute("dexterity").as_int();

		playerAttributes[playerID].currentHealth = playerAttributes[playerID].maxHealth;
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