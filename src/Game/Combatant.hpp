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


class Combatant
{
public:
	virtual void Init(int _id) = 0;

	void SetPos(int _x, int _y);
	sf::IntRect &GetRect() { return hitbox; }

	~Combatant();

protected:

	SpriterEngine::EntityInstance *combatantObject;
	sf::IntRect hitbox;

	combatantAttributes attributes;


};