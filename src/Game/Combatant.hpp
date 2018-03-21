#pragma once

#include "../Framework/Animations/sfml_implementation/exampleobjectfactory.h"
#include "../Framework/Animations/sfml_implementation/examplefilefactory.h"

// include to use Spriter animations
#include "../Framework/Animations/spriterengine.h"
#include "../Framework/Animations/global/settings.h"


#include "../Framework/Makros.hpp"

#include "../Framework/Gui/Bar.hpp"
#include "Resources\SpritePool.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "CombatantStatus.hpp"

class Combatant
{
public:
	virtual void Init(int _id) = 0;
	void Quit();
	virtual void Render() = 0;

	void SetPos(int _x, int _y);
	sf::IntRect &GetRect() { return hitbox; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) { return true; }
	virtual int GetID() { return -2; }

	CombatantStatus &Status() { return status; }

	int GetBattlePos() { return battlePosition; }
	void SetBattlePos(int _pos) { battlePosition = _pos; }

	virtual void RenderHealthBar(sf::RenderTarget &_target);

	virtual bool IsPlayer() { return false; }

	PossibleAims possibleAbilityAims[4];

protected:

	SpriterEngine::EntityInstance *combatantObject;
	sf::IntRect hitbox;

	CombatantStatus status;

	int battlePosition;

	Bar healthBar;

	void RenderStatusSymbols(sf::RenderTarget &_target);
};