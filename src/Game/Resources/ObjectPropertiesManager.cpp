#include "ObjectPropertiesManager.hpp"




void ObjectPropertiesManager::LoadObjectProperties()
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