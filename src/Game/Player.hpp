#pragma once

#include "Combatant.hpp"
#include "../Framework/Gameengine.hpp"
#include "Resources\Resources.hpp"


const float WALKING_ANIMATION_SPEED = 0.85f;

class Player : public Combatant
{
public:
	Player(CombatantID _id, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, LevelStatus *_levelStatus, SpriterEngine::EntityInstance *_combatantObject);

	void Update(int _xMove, bool _is_walking);

	virtual bool IsPlayer() override { return true; }

	void SetEquipment(CombatantAttributes &_equipmentStats);

	float GetAdditionalDamageForCurrentlyAimedCombatant();

	void SetAbilityStatus(abilityPhase _status) override;

	bool CurrentAbilityCanAimAtCombatant(Combatant *_combatant);


protected:

	bool is_walking;
	LevelStatus* levelStatus;
};