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
#include "../Framework/Time.hpp"
#include "CombatantStatus.hpp"


#include "BattleGUI.hpp"


enum abilityPhase { ready, executing, finished, attacked };

class Combatant
{
public:
	virtual void Init(int _id, CGameEngine *_engine) = 0;
	void Quit();
	virtual void Render() = 0;
	virtual void Update();

	void SetPos(int _x, int _y);
	sf::IntRect &GetRect() { return hitbox; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) { return true; }
	virtual int GetID() { return -2; }

	void GiveTurnTo(std::vector<Combatant*> *_targets, BattleGUI *_gui);

	void StartAttackedAnimation();
	void StopAttackedAnimation();

	bool FinishedTurn() { return abilityStatus == finished; }

	CombatantStatus &Status() { return status; }

	int GetBattlePos() { return battlePosition; }
	void SetBattlePos(int _pos) { battlePosition = _pos; }

	abilityPhase GetAbilityStatus() { return abilityStatus; }

	virtual void RenderHealthBar(sf::RenderTarget &_target);

	virtual bool IsPlayer() { return false; }

	PossibleAims possibleAbilityAims[4];

protected:

	SpriterEngine::EntityInstance *combatantObject;
	sf::IntRect hitbox;

	CombatantStatus status;

	int battlePosition;
	SpriterEngine::point lastPosition;

	Bar healthBar;

	CGameEngine *engine;
	BattleGUI *gui;

	abilityPhase abilityStatus;

	std::vector<Combatant*> *allCombatants;

	std::vector<Combatant*> selectedTargets;

	void RenderStatusSymbols(sf::RenderTarget &_target);

	void StartTargetsAttackedAnimation();
	void StopTargetsAttackedAnimation();
};