#include "PlayerStatePrepareAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "CombatantStateExecutingAbility.hpp"
#include "Markus.hpp"

PlayerStatePrepareAbility::PlayerStatePrepareAbility(Player * _context)
	:CombatantStatePrepareAbility(_context)
{
	playerContext = _context;
}


void PlayerStatePrepareAbility::Update()
{
	UpdateSelectedTargets();

	if (playerContext->engine->GetButtonstates(ButtonID::Left) == Keystates::Released && !playerContext->selectedTargets.empty())
	{
		HandleConfusion();

		if (playerContext->selectedTargets.empty())
		{
			CombatantStateIdle *newState = new CombatantStateIdle(context);
			context->ChangeState(newState);
			return;
		}

		CombatantStateExecutingAbility *newState = new CombatantStateExecutingAbility(playerContext, &g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()]);
		context->ChangeState(newState);
		return;
	}
}

bool PlayerStatePrepareAbility::CurrentAbilityCanAimAtCombatant(Combatant* _combatant)
{
	bool canAim = g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].possibleAims.position[_combatant->GetBattlePos()] && !_combatant->IsDying();

	if (dynamic_cast<PlayerMarkus*>(playerContext) != nullptr && playerContext->gui->GetCurrentAbility() == 0) {
		auto *markus = dynamic_cast<PlayerMarkus*>(playerContext);
		return (canAim && markus->CanAimFistOfRevengeAt(_combatant->GetBattlePos()));
	}

	return canAim;
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

void PlayerStatePrepareAbility::HandleConfusion()
{
	if (playerContext->actsInConfusion)
	{
		bool originallyAttackedPlayer = playerContext->selectedTargets[0]->IsPlayer();
		playerContext->selectedTargets.clear();
		playerContext->notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_kingArthur, sf::Vector2f(playerContext->GetRect().left - playerContext->GetRect().width / 2.0f, playerContext->GetRect().top - 20.0f), 1.0f);
		
		if (originallyAttackedPlayer)
			ChooseRandomOpponent();
		else
			ChooseRandomAlly();

		SelectAdditionalTargets(!originallyAttackedPlayer);
	}
}



bool PlayerStatePrepareAbility::PlayerShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition)
{
	if (!_combatant->IsPlayer() || _combatant == playerContext)
		return false;

	return CurrentAbilityAttacksAll() && playerContext != _combatant  && playerContext->selectedTargets[0] != _combatant
		|| _combatant->GetBattlePos() < _targetPosition && _combatant->GetBattlePos() > _targetPosition - NumberOfTargetsForCurrentAbility();
}

bool PlayerStatePrepareAbility::EnemyShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition)
{
	if (_combatant->IsPlayer())
		return false;
	
	return CurrentAbilityAttacksAllPlayers() && playerContext->selectedTargets[0] != _combatant
		|| CurrentAbilityAttacksAll() && playerContext != _combatant  && playerContext->selectedTargets[0] != _combatant
		|| _combatant->GetBattlePos() > _targetPosition && _combatant->GetBattlePos() < _targetPosition + NumberOfTargetsForCurrentAbility();
}


void PlayerStatePrepareAbility::SelectAdditionalTargets(bool _selectPlayers)
{
	if (playerContext->selectedTargets.empty())
		return;

	int targetPosition = playerContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*playerContext->allCombatants))
	{
		if (_selectPlayers && PlayerShouldBeAddedAsTarget(c, targetPosition) || !_selectPlayers && !EnemyShouldBeAddedAsTarget(c, targetPosition))
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
	for (Combatant *c : playerContext->selectedTargets)
		c->RenderAbilityTargetMarker();
}

void PlayerStatePrepareAbility::UpdateSelectedTargets()
{
	playerContext->selectedTargets.clear();
	for (Combatant *c : (*playerContext->allCombatants))
	{
		if (c->GetRect().contains(playerContext->engine->GetWorldMousePos()) && CurrentAbilityCanAimAtCombatant(c))
		{
			int targetPosition = c->GetBattlePos();

			for (Combatant* com : (*playerContext->allCombatants))
			{
				if (CurrentAbilityAttacksAllPlayers() && c->IsPlayer() && com->IsPlayer()
					|| CurrentAbilityAttacksAll() && playerContext != com
					|| com->GetBattlePos() >= targetPosition && com->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
						playerContext->selectedTargets.push_back(com);
			}

			return;
		}
	}
}


float PlayerStatePrepareAbility::GetAdditionalDamageForCurrentlyAimedCombatant()
{
	int numberOfCombatants = playerContext->selectedTargets.size();
	float additionalDamage = g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].effectHostile.lessTargetsMoreDamage * (NumberOfTargetsForCurrentAbility() - numberOfCombatants);
	return additionalDamage;
}