#pragma once

#include "../Framework/Animations/sfml_implementation/exampleobjectfactory.h"
#include "../Framework/Animations/sfml_implementation/examplefilefactory.h"

// include to use Spriter animations
#include "../Framework/Animations/spriterengine.h"
#include "../Framework/Animations/global/settings.h"


#include "../Framework/Makros.hpp"



struct combatantAttributes
{
	int maxHealth;
	int currentHealth;
	int damage;
	int armour;
	int initiative;
};

struct PossibleAims
{
	bool position[8];
	int howMany;
};

class Combatant
{
public:
	virtual void Init(int _id) = 0;
	void Quit();

	void SetPos(int _x, int _y);
	sf::IntRect &GetRect() { return hitbox; }
	combatantAttributes &GetAttributes() { return attributes; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) { return true; }
	virtual int GetID() { return -2; }

	virtual bool IsPlayer() { return false; }

	PossibleAims possibleAbilityAims[4];

protected:

	SpriterEngine::EntityInstance *combatantObject;
	sf::IntRect hitbox;

	combatantAttributes attributes;


};