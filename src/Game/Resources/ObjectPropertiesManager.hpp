#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include "../../Framework/Animations/pugixml/pugixml.hpp"

#include "../CombatantAttributes.hpp"


struct PossibleAims
{
	bool position[8];
	int howMany;
};

#define g_pObjectProperties ObjectPropertiesManager::Get()
class ObjectPropertiesManager : public TSingleton<ObjectPropertiesManager>
{
public:
	void LoadObjectProperties();

	PossibleAims playerAbilities[4][4];
	CombatantAttributes playerAttributes[4];

private:
	void LoadPlayerAbilities();
	void LoadPlayerAttributes();
};

