#include "Player.hpp"
#include "Markus.hpp"

Player::Player(int _id, CGameEngine * _engine, NotificationRenderer * _notificationRenderer)
	: Combatant(_id, _engine, _notificationRenderer)
{
}

void Player::Init()
{
	status.SetStats(g_pObjectProperties->playerStats[GetID()]);

	Combatant::Init();

	is_walking = false;
}


void Player::SetEquipment(CombatantAttributes &_equipmentStats)
{
	status.AddStats(_equipmentStats);
}

float Player::GetAdditionalDamageForCurrentlyAimedCombatant()
{
	int numberOfCombatants = 0;
	for (Combatant *c : (*allCombatants))
	{
		if (c->GetRect().contains(engine->GetWorldMousePos()) && CurrentAbilityCanAimAtCombatant(c))
		{
			int targetPosition = c->GetBattlePos();

			for (Combatant* c : (*allCombatants))
			{
				if (CurrentAbilityAttacksAll() && this != c || c->GetBattlePos() >= targetPosition && c->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
					numberOfCombatants++;
			}

			break;
		}
	}

	float additionalDamage = g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].effectHostile.lessTargetsMoreDamage * (NumberOfTargetsForCurrentAbility() - numberOfCombatants);
	return additionalDamage;
}


int Player::NumberOfTargetsForCurrentAbility()
{
	return g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].possibleAims.howMany;
}

bool Player::CurrentAbilityAttacksAll()
{
	return g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].possibleAims.attackAll;
}

bool Player::CurrentAbilityAttacksAllPlayers()
{
	return g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].possibleAims.attackAllPlayers;
}


void Player::Update(int _xMove, bool _is_walking)
{
	Combatant::Update();
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

	if (abilityStatus == handlingStatus)
	{
		Status().ExecuteStatusChanges();
		if (!Status().IsExecutingStatusChanges())
		{
			if (Status().SkipRound())
				abilityStatus = finished;
			else
				abilityStatus = ready;
		}

	}

	if (abilityStatus == ready && AimChosen())
	{
		abilityStatus = executing;
		StartAbilityAnimation(gui->GetCurrentAbility());
		StartTargetsAttackedAnimation(g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].precisionModificator);
	}
	
	if (abilityStatus == executing)
		DoCurrentAbility();
}



void Player::Render()
{
	if(abilityStatus != executing && abilityStatus != attacked && abilityStatus != dodging)
		RenderShadow();

	combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	combatantObject->render();
	combatantObject->playSoundTriggers();

	if (abilityStatus == attacked || abilityStatus == dodging)
		RenderAbilityEffects();

	if (abilityStatus == ready || abilityStatus == handlingStatus)
		RenderTurnMarker();
	
	if(abilityStatus == ready)
		RenderAbilityTargetMarker();

	if (abilityStatus != executing && abilityStatus != attacked && abilityStatus != dodging && !IsDying())
		statusBar.Render();

}


void Player::RenderAbilityTargetMarker()
{
	for (Combatant *c : (*allCombatants))
	{
		if (c->GetRect().contains(engine->GetWorldMousePos()) && CurrentAbilityCanAimAtCombatant(c))
		{
			int targetPosition = c->GetBattlePos();

			for (Combatant* com : (*allCombatants))
			{
				if (CurrentAbilityAttacksAllPlayers())
				{
					if (c->IsPlayer() && com->IsPlayer() || !c->IsPlayer() && com->GetBattlePos() >= targetPosition && com->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
						com->RenderAbilityTargetMarker();
				}
				else if (CurrentAbilityAttacksAll() && this != com 
					|| com->GetBattlePos() >= targetPosition && com->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
					com->RenderAbilityTargetMarker();				
			}

			return;
		}
	}
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
		if (CurrentAbilityAttacksAllPlayers() && selectedTargets[0]->IsPlayer())
		{
			if (selectedTargets[0] != c && c->IsPlayer())
				selectedTargets.push_back(c);
		}
		else if (CurrentAbilityAttacksAll() && this != c  && selectedTargets[0] != c 
			|| c->GetBattlePos() > targetPosition && c->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
			selectedTargets.push_back(c);
	}
}



bool Player::CurrentAbilityCanAimAtCombatant(Combatant* _combatant)
{
	bool canAim = g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()].possibleAims.position[_combatant->GetBattlePos()] && !_combatant->IsDying();

	if (dynamic_cast<PlayerMarkus*>(this) != nullptr && gui->GetCurrentAbility() == 0) { 
		auto *markus = dynamic_cast<PlayerMarkus*>(this);
		return (canAim && markus->CanAimFistOfRevengeAt(_combatant->GetBattlePos()));
	}

	return canAim;
}



bool Player::CombatantClicked(Combatant* _combatant)
{
	return _combatant->GetRect().contains(engine->GetWorldMousePos());
}



void Player::DoCurrentAbility()
{
	if (!combatantObject->animationIsPlaying() && !AbilityEffectIsPlaying())
	{
		HandleConfusion();

		auto &ability = g_pObjectProperties->playerAbilities[GetID()][gui->GetCurrentAbility()];

		for (Combatant *t : selectedTargets)
		{
			if (t->IsPlayer() && !actsInConfusion)
			{
				float additionalDamage = ability.effectFriendly.lessTargetsMoreDamage * (ability.possibleAims.howMany - selectedTargets.size());
				ApplyAbilityEffectToTarget(t, ability.effectFriendly, additionalDamage);
			}
			else if (t->GetAbilityStatus() != dodging)
			{
				float additionalDamage = ability.effectHostile.lessTargetsMoreDamage * (ability.possibleAims.howMany - selectedTargets.size());
				ApplyAbilityEffectToTarget(t, ability.effectHostile, additionalDamage);
			}
		}

		SetAnimation("idle", IDLE_ANIMATION_SPEED);
		ReverseScaleForAbilityAnimation();
		StopTargetsAttackedAnimation();
		
		selectedTargets.clear();
		abilityStatus = finished;

		if (dynamic_cast<PlayerMarkus*>(this))
			dynamic_cast<PlayerMarkus*>(this)->ResetFistOfRevenge();
	}
}



void Player::HandleConfusion()
{
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1, false);
	}
}



void Player::StartAbilityAnimation(int _ability)
{
	ScaleForAbilityAnimation();

	SetAnimation(g_pObjectProperties->playerAbilities[GetID()][_ability].animation, ABILITY_ANIMATION_SPEED);

	if (selectedTargets[0]->IsPlayer())
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->playerAbilities[GetID()][_ability].effectAnimationFriendly);
	else
		g_pSpritePool->abilityEffectsAnimation->setCurrentAnimation(g_pObjectProperties->playerAbilities[GetID()][_ability].effectAnimationHostile);

	g_pSpritePool->abilityEffectsAnimation->setCurrentTime(0);
}