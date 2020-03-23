#pragma once

#include "../Framework/Animations/sfml_implementation/exampleobjectfactory.h"
#include "../Framework/Animations/sfml_implementation/examplefilefactory.h"

// include to use Spriter animations
#include "../Framework/Animations/spriterengine.h"
#include "../Framework/Animations/global/settings.h"


#include "../Framework/Makros.hpp"
#include "../Framework/Patterns/Observer.hpp"

#include "Resources\ObjectPropertiesManager.hpp"
#include "../Framework/Time.hpp"
#include "CombatantStatus.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"

#include "CombatantStatusBar.hpp"
#include "CombatantID.hpp"

#include "BattleGUI.hpp"


enum abilityPhase { ready, executing, finished, attacked, dodging, handlingStatus };

const float IDLE_ANIMATION_SPEED = 0.7f;
const float ABILITY_ANIMATION_SPEED = 1.0f;
const float ABILITY_EFFECT_ANIMATION_SPEED = 0.9f;

const float COMBATANT_NORMAL_SCALE = 0.6f;
const float COMBATANT_ABILITY_SCALE = 0.8f;

const float TURN_MARKER_ANIMATION_SCALE = 1.2f;

class Combatant : public CObserver
{
public:
	static bool setElapsedTimeForAbilityEffect;

	Combatant(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual void Init();
	void Quit();
	virtual void Render() = 0;
	virtual void Update();

	virtual int GetID() { return -2; }
	virtual bool IsPlayer() { return false; }
	sf::IntRect &GetRect() { return hitbox; }
	sf::Vector2f GetLocalPosition();

	CombatantStatus &Status() { return status; }
	abilityPhase GetAbilityStatus() { return abilityStatus; }

	bool FinishedTurn() { return abilityStatus == finished; }
	bool IsDying() { return dying; }
	bool AnimationFinished() { return !combatantObject->animationIsPlaying(); }


	void SetPos(int _x, int _y);
	void Scale(float _x, float _y);

	virtual bool DoAbility(int _id, std::vector<Combatant*> &_targets) { return true; }
	void GiveTurnTo(std::vector<Combatant*> *_targets, BattleGUI *_gui);

	void StartAttackedAnimation();
	void StartDodgingAnimation();
	void StartFriendlyAttackedAnimation();
	void StopAttackedAnimation();
	void StartDeathAnimation();

	int GetBattlePos() { return battlePosition; }
	void SetBattlePos(int _pos) { battlePosition = _pos; }

	void RenderAbilityTargetMarker();
	void RenderTurnMarker();

	void ResetAbilityStatus() { abilityStatus = finished; }
	void SetAnimation(std::string _animation, float _speed);

protected:

	CGameEngine *engine;
	BattleGUI *gui;
	NotificationRenderer *notificationRenderer;

	std::vector<Combatant*> *allCombatants;
	std::vector<Combatant*> selectedTargets;

	SpriterEngine::EntityInstance *combatantObject;
	sf::IntRect hitbox;
	SpriterEngine::point abilityEffectPoint;

	CombatantStatus status;
	abilityPhase abilityStatus;

	CombatantStatusBar statusBar;

	int battlePosition;
	SpriterEngine::point lastPosition;

	bool actsInConfusion;
	bool dying;

	float turnMarkerScale;

	void RenderAbilityEffects();
	void RenderShadow();

	virtual void ApplyAbilityEffectToTarget(Combatant *_target, AbilityEffect &_effect, float _additionalDamageFactor = 0.0f);

	void StartTargetsAttackedAnimation(int _abilityPrecision);
	void StopTargetsAttackedAnimation();

	void ScaleForAbilityAnimation();
	void ReverseScaleForAbilityAnimation();

	void ReloadHitbox();
	void ReloadAbilityEffectPoint();

	bool AbilityEffectIsPlaying();
	bool CheckForDodging(Combatant *_attacker, int _precisionModificator);

	void ChooseRandomAlly();
	void ChooseRandomOpponent();

	void HandleConfusion();

	bool IsAlly(Combatant *c);
};