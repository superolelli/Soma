#include "Player.hpp"



void Player::Init(int _id, CGameEngine *_engine)
{

	engine = _engine;

	SetAnimation("idle", IDLE_ANIMATION_SPEED);

	combatantObject->setScale(SpriterEngine::point(PLAYER_SCALE, PLAYER_SCALE));
	combatantObject->reprocessCurrentTime();

	possibleAbilityAims[0] = g_pObjectProperties->playerAbilities[_id][0];
	possibleAbilityAims[1] = g_pObjectProperties->playerAbilities[_id][1];
	possibleAbilityAims[2] = g_pObjectProperties->playerAbilities[_id][2];
	possibleAbilityAims[3] = g_pObjectProperties->playerAbilities[_id][3];

	status.SetAttributes(g_pObjectProperties->playerAttributes[_id]);

	SpriterEngine::UniversalObjectInterface* hitboxObj = combatantObject->getObjectInstance("bounding_box");

	hitbox.width = hitboxObj->getSize().x * combatantObject->getScale().x;
	hitbox.height = hitboxObj->getSize().y * combatantObject->getScale().y;
	hitbox.left = hitboxObj->getPosition().x;
	hitbox.top = hitboxObj->getPosition().y;

	healthBar.Load(g_pTextures->healthBar, g_pTextures->healthBarFrame, status.GetCurrentHealthPointer(), status.GetMaxHealthPointer());
	healthBar.SetPos(GetRect().left + GetRect().width / 2 - healthBar.GetRect().width / 2, GetRect().top + GetRect().height + 30);
	healthBar.SetSmoothTransformationTime(0.7f);

	status.Reset();

	is_walking = false;

	abilityStatus = finished;
}




void Player::Update(int _xMove, bool _is_walking)
{
	SetPos(combatantObject->getPosition().x + _xMove, combatantObject->getPosition().y);

	if (abilityStatus != executing && abilityStatus != attacked)
	{
		if (is_walking == false && _is_walking == true)
		{
			is_walking = true;
			SetAnimation("walk", WALKING_ANIMATION_SPEED);
		}
		else if (is_walking == true && _is_walking == false)
		{
			is_walking = false;
			SetAnimation("idle", IDLE_ANIMATION_SPEED);
		}
	}


	if (abilityStatus == ready && AimChosen())
	{
		abilityStatus = executing;
		StartAbilityAnimation(gui->GetCurrentAbility());
		StartTargetsAttackedAnimation();
	}

	if (abilityStatus == executing)
		DoCurrentAbility();
}



void Player::Render()
{
	combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	combatantObject->render();
}



bool Player::AimChosen()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Released)
	{
		for (Combatant* c : (*allCombatants))
		{
			if (CurrentAbilityCanAimAtCombatant(c) && CombatantClicked(c))
			{
				selectedTargets.push_back(c);
				SelectAdditionalTargets();

				return true;
			}
		}
	}

	return false;
}



void Player::SelectAdditionalTargets()
{
	int targetPosition = selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*allCombatants))
	{
		if (c->GetBattlePos() > targetPosition && c->GetBattlePos() < targetPosition + possibleAbilityAims[gui->GetCurrentAbility()].howMany)
			selectedTargets.push_back(c);
	}
}



bool Player::CurrentAbilityCanAimAtCombatant(Combatant* _combatant)
{
	return possibleAbilityAims[gui->GetCurrentAbility()].position[_combatant->GetBattlePos()];
}



bool Player::CombatantClicked(Combatant* _combatant)
{
	return _combatant->GetRect().contains(engine->GetWorldMousePos());
}



void Player::DoCurrentAbility()
{
	if (combatantObject->animationJustFinished())
	{
		SetAnimation("idle", IDLE_ANIMATION_SPEED);
		ReverseScaleForAbilityAnimation();
		StopTargetsAttackedAnimation();

		DoAbility(gui->GetCurrentAbility(), selectedTargets);
		selectedTargets.clear();
		abilityStatus = finished;
	}
}


