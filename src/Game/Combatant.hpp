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

class CombatantState;
class Battle;

enum abilityPhase { ready, executing, finished, attacked, dodging, handlingStatus, dying };

const float IDLE_ANIMATION_SPEED = 0.7f;
const float ABILITY_ANIMATION_SPEED = 1.0f;
const float ABILITY_EFFECT_ANIMATION_SPEED = 0.9f;

const float COMBATANT_NORMAL_SCALE = 0.6f;
const float COMBATANT_ABILITY_SCALE = 0.8f;

const float TURN_MARKER_ANIMATION_SCALE = 1.2f;

class Combatant : public CObserver
{
	friend class CombatantStateAttacked;
	friend class CombatantStateUpdateStatus;
	friend class CombatantStateIdle;
	friend class CombatantStateExecutingAbility;
	friend class EnemyStatePrepareAbility;
	friend class PlayerStatePrepareAbility;
	friend class CombatantStatePrepareAbility;
	friend class CombatantStateDying;

public:
	static bool setElapsedTimeForAbilityEffect;

	Combatant(int _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer);

	virtual void Init();
	void Quit();
	virtual void Update();

	virtual void Render() = 0;
	virtual int GetID() = 0;
	virtual bool IsPlayer() = 0;
	virtual void SetAbilityStatus(abilityPhase _status) = 0;

	void ChangeState(CombatantState *_state);
	void SetBattlePtr(Battle *_battle);

	sf::IntRect &GetRect() { return hitbox; }
	sf::Vector2f GetLocalPosition();

	CombatantStatus &Status() { return status; }
	virtual abilityPhase GetAbilityStatus();

	bool FinishedTurn() { return GetAbilityStatus() == finished || GetAbilityStatus() == dying; }
	bool IsDying() { return GetAbilityStatus() == dying; }
	bool AnimationFinished() { return !combatantObject->animationIsPlaying(); }


	void SetPos(int _x, int _y);
	void Scale(float _x, float _y);

	int GetBattlePos() { return battlePosition; }
	void SetBattlePos(int _pos) { battlePosition = _pos; }

	void SetAnimation(const std::string &_animation, float _speed);
	void ResetAbilityStatus() { SetAbilityStatus(finished); }

	void GiveTurnTo(std::vector<Combatant*> *_targets, BattleGUI *_gui);
	void SetTurnPending(bool _pending);

	void StartAttackedAnimation();
	void StartDodgingAnimation();
	void StartFriendlyAttackedAnimation();
	void StopAttackedAnimation();
	void StartDeathAnimation();


protected:

	CombatantState *currentState;
	CombatantStatus status;
	CombatantStatusBar statusBar;
	SpriterEngine::EntityInstance *combatantObject;
	sf::IntRect hitbox;

	CGameEngine *engine;
	BattleGUI *gui;
	NotificationRenderer *notificationRenderer;
	Battle *battle;

	std::vector<Combatant*> *allCombatants;
	std::vector<Combatant*> selectedTargets;

	int battlePosition;
	SpriterEngine::point lastPosition;

	bool actsInConfusion;
	float turnMarkerScale;

	void RenderShadow();
	void RenderAbilityTargetMarker();
	void RenderTurnMarker();

	void ApplyAbilityEffect(Combatant *_attacker, AbilityEffect &_effect, float _additionalDamageFactor = 0.0f);

	void ScaleForAbilityAnimation();
	void ReverseScaleForAbilityAnimation();

	void ReloadHitbox();

	bool IsAlly(Combatant *c);
};