#include "PlayerStatePrepareAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "PlayerStateExecutingAbility.hpp"
#include "Markus.hpp"

PlayerStatePrepareAbility::PlayerStatePrepareAbility(Player * _context)
	:PlayerState(_context){}

void PlayerStatePrepareAbility::Update()
{
	if (playerContext->engine->GetButtonstates(ButtonID::Left) == Keystates::Released)
	{
		for (Combatant* c : (*playerContext->allCombatants))
		{
			if (CurrentAbilityCanAimAtCombatant(c) && CombatantClicked(c))
			{		
				PrepareTargets(c);
				return;
			}
		}
	}
}

bool PlayerStatePrepareAbility::CurrentAbilityCanAimAtCombatant(Combatant* _combatant)
{
	bool canAim = g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].possibleAims.position[_combatant->GetBattlePos()] && !_combatant->IsDying();

	if (dynamic_cast<PlayerMarkus*>(this) != nullptr && playerContext->gui->GetCurrentAbility() == 0) {
		auto *markus = dynamic_cast<PlayerMarkus*>(this);
		return (canAim && markus->CanAimFistOfRevengeAt(_combatant->GetBattlePos()));
	}

	return canAim;
}


bool PlayerStatePrepareAbility::CombatantClicked(Combatant* _combatant)
{
	return _combatant->GetRect().contains(playerContext->engine->GetWorldMousePos());
}

void PlayerStatePrepareAbility::PrepareTargets(Combatant *_target)
{
	if (playerContext->actsInConfusion)
	{
		playerContext->notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_kingArthur, sf::Vector2f(playerContext->GetRect().left - playerContext->GetRect().width / 2.0f, playerContext->GetRect().top - 20.0f), 1.0f);
		ChooseCombatantsForConfusionAttack(_target);

		if (playerContext->selectedTargets.empty())
		{
			CombatantStateIdle *newState = new CombatantStateIdle(context);
			context->ChangeState(newState);
			return;
		}
	}
	else
	{
		playerContext->selectedTargets.push_back(_target);
		SelectAdditionalTargets();
	}

	PlayerStateExecutingAbility *newState = new PlayerStateExecutingAbility(playerContext);
	context->ChangeState(newState);
}



void PlayerStatePrepareAbility::SelectAdditionalTargets()
{
	int targetPosition = playerContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*playerContext->allCombatants))
	{
		if (CurrentAbilityAttacksAllPlayers() && playerContext->selectedTargets[0]->IsPlayer())
		{
			if (playerContext->selectedTargets[0] != c && c->IsPlayer())
				playerContext->selectedTargets.push_back(c);
		}
		else if (CurrentAbilityAttacksAll() && playerContext != c  && playerContext->selectedTargets[0] != c
			|| c->GetBattlePos() > targetPosition && c->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
			playerContext->selectedTargets.push_back(c);
	}
}

int PlayerStatePrepareAbility::NumberOfTargetsForCurrentAbility()
{
	return g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].possibleAims.howMany;
}

bool PlayerStatePrepareAbility::CurrentAbilityAttacksAll()
{
	return g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].possibleAims.attackAll;
}

bool PlayerStatePrepareAbility::CurrentAbilityAttacksAllPlayers()
{
	return g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].possibleAims.attackAllPlayers;
}

void PlayerStatePrepareAbility::ChooseCombatantsForConfusionAttack(Combatant *_originallyAttackedCombatant)
{
	if (_originallyAttackedCombatant->IsPlayer())
		playerContext->ChooseRandomOpponent();
	else
		playerContext->ChooseRandomAlly();

	if (!playerContext->selectedTargets.empty())
	{
		if (_originallyAttackedCombatant->IsPlayer())
			SelectAdditionalEnemies();
		else
			SelectAdditionalPlayers();
	}
}

void PlayerStatePrepareAbility::SelectAdditionalPlayers()
{
	int targetPosition = playerContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*playerContext->allCombatants))
	{
		if (c == playerContext)
			continue;

		if (CurrentAbilityAttacksAll() && playerContext != c  && playerContext->selectedTargets[0] != c
			|| c->GetBattlePos() < targetPosition && c->GetBattlePos() > targetPosition - NumberOfTargetsForCurrentAbility())
			playerContext->selectedTargets.push_back(c);
	}
}

void PlayerStatePrepareAbility::SelectAdditionalEnemies()
{
	int targetPosition = playerContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*playerContext->allCombatants))
	{
		if (CurrentAbilityAttacksAllPlayers())
		{
			if (playerContext->selectedTargets[0] != c && !c->IsPlayer())
				playerContext->selectedTargets.push_back(c);
		}
		else if (CurrentAbilityAttacksAll() && playerContext != c  && playerContext->selectedTargets[0] != c
			|| c->GetBattlePos() > targetPosition && c->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
			playerContext->selectedTargets.push_back(c);
	}
}


void PlayerStatePrepareAbility::Render()
{
	playerContext->RenderShadow();

	playerContext->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	playerContext->combatantObject->render();
	playerContext->combatantObject->playSoundTriggers();

	playerContext->RenderTurnMarker();
	RenderAbilityTargetMarker();

	playerContext->statusBar.Render();
}


void PlayerStatePrepareAbility::RenderAbilityTargetMarker()
{
	for (Combatant *c : (*playerContext->allCombatants))
	{
		if (c->GetRect().contains(playerContext->engine->GetWorldMousePos()) && CurrentAbilityCanAimAtCombatant(c))
		{
			int targetPosition = c->GetBattlePos();

			for (Combatant* com : (*playerContext->allCombatants))
			{
				if (CurrentAbilityAttacksAllPlayers())
				{
					if (c->IsPlayer() && com->IsPlayer() || !c->IsPlayer() && com->GetBattlePos() >= targetPosition && com->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
						com->RenderAbilityTargetMarker();
				}
				else if (CurrentAbilityAttacksAll() && playerContext != com
					|| com->GetBattlePos() >= targetPosition && com->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
					com->RenderAbilityTargetMarker();
			}

			return;
		}
	}
}


float PlayerStatePrepareAbility::GetAdditionalDamageForCurrentlyAimedCombatant()
{
	int numberOfCombatants = 0;
	for (Combatant *c : (*playerContext->allCombatants))
	{
		if (c->GetRect().contains(playerContext->engine->GetWorldMousePos()) && CurrentAbilityCanAimAtCombatant(c))
		{
			int targetPosition = c->GetBattlePos();

			for (Combatant* c : (*playerContext->allCombatants))
			{
				if (CurrentAbilityAttacksAll() && playerContext != c || c->GetBattlePos() >= targetPosition && c->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
					numberOfCombatants++;
			}

			break;
		}
	}

	float additionalDamage = g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].effectHostile.lessTargetsMoreDamage * (NumberOfTargetsForCurrentAbility() - numberOfCombatants);
	return additionalDamage;
}