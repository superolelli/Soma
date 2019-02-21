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
#include "../Framework/Graphics/NotificationRenderer.hpp"


#include "BattleGUI.hpp"


enum abilityPhase { ready, executing, finished, attacked, dodging };

const float IDLE_ANIMATION_SPEED = 0.7f;
const float ABILITY_ANIMATION_SPEED = 1.0f;
const float ABILITY_EFFECT_ANIMATION_SPEED = 0.9f;

class Combatant
{
public:
	static bool setElapsedTimeForAbilityEffect;

	virtual void Init(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer) = 0;
	void Quit();
	virtual void Render() = 0;
	virtual void Update();

	void SetPos(int _x, int _y);
	void Scale(float _x, float _y);
	sf::IntRect &GetRect() { return hitbox; }

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) { return true; }
	virtual int GetID() { return -2; }

	void GiveTurnTo(std::vector<Combatant*> *_targets, BattleGUI *_gui);

	void StartAttackedAnimation();
	void StartDodgingAnimation();
	void StartFriendlyAttackedAnimation();
	void StopAttackedAnimation();

	bool FinishedTurn() { return abilityStatus == finished; }

	CombatantStatus &Status() { return status; }

	int GetBattlePos() { return battlePosition; }
	void SetBattlePos(int _pos) { battlePosition = _pos; }

	abilityPhase GetAbilityStatus() { return abilityStatus; }

	virtual void RenderHealthBar(sf::RenderTarget &_target);
	void RenderAbilityTargetMarker();
	void RenderTurnMarker();

	void ResetAbilityStatus() { abilityStatus = finished; }

	virtual bool IsPlayer() { return false; }

protected:

	SpriterEngine::EntityInstance *combatantObject;
	sf::IntRect hitbox;
	SpriterEngine::point abilityEffectPoint;

	CombatantStatus status;

	int battlePosition;
	SpriterEngine::point lastPosition;

	bool actsInConfusion;

	Bar healthBar;

	CGameEngine *engine;
	BattleGUI *gui;
	NotificationRenderer *notificationRenderer;

	abilityPhase abilityStatus;

	std::vector<Combatant*> *allCombatants;

	std::vector<Combatant*> selectedTargets;

	void RenderStatusSymbols(sf::RenderTarget &_target);
	void RenderStatusSymbolsTooltips();

	void RenderAbilityEffects();

	void ApplyAbilityEffectToTarget(Combatant *_target, AbilityEffect &_effect);

	void StartTargetsAttackedAnimation();
	void StopTargetsAttackedAnimation();

	void ScaleForAbilityAnimation();
	void ReverseScaleForAbilityAnimation();

	void SetAnimation(std::string _animation, float _speed);

	void ReloadHitbox();
	void ReloadAbilityEffectPoint();

	bool AbilityEffectIsPlaying();
	bool CheckForDodging(Combatant *_attacker);

	void RenderTooltip(std::string _tooltip, float _x, float _y);
};