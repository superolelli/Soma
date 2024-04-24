#include "PlayerStatePrepareAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "CombatantStateExecutingAbility.hpp"
#include "Markus.hpp"
#include "Battle.hpp"

PlayerStatePrepareAbility::PlayerStatePrepareAbility(Player * _context)
	:CombatantStatePrepareAbility(_context)
{
	playerContext = _context;
	notificationWaitingTime = 0.0;
}


void PlayerStatePrepareAbility::Update()
{
	if (notificationWaitingTime > 0.0)
	{
		notificationWaitingTime -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();

		if (notificationWaitingTime <= 0.0)
			ChangeState();
	}
	else
	{
		UpdateSelectedTargets();

		if (playerContext->gui->ShouldSkipTurn())
		{
			if (playerContext->Status().HasHealOnPass()) 
			{
				playerContext->battle->GetLevelStatus()->AddFatigue(3);
				playerContext->Status().GainHealth(3);
			}

			playerContext->selectedTargets.clear();
			ChangeState();
		}

		if (playerContext->engine->GetButtonstates(ButtonID::Left) == Keystates::Released && !playerContext->selectedTargets.empty())
		{
			if (playerContext->Status().GetFatigueLevel() == CombatantStatus::FatigueLevel::stupid)
				HandleStupidness();
			else if (playerContext->actsInConfusion)
				HandleConfusion();
			else
				ChangeState();
		}
	}
}


void PlayerStatePrepareAbility::ChangeState()
{
	if (playerContext->selectedTargets.empty())
	{
		CombatantStateIdle *newState = new CombatantStateIdle(context);
		context->ChangeState(newState);
		return;
	}

	CombatantStateExecutingAbility *newState = new CombatantStateExecutingAbility(playerContext, &g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()]);
	context->ChangeState(newState);
}

bool PlayerStatePrepareAbility::CurrentAbilityCanAimAtCombatant(Combatant* _combatant)
{
	bool canAim = g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].possibleAims.position[_combatant->GetBattlePos()] && !_combatant->IsDying() ||
		g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].possibleAims.attackSelf && _combatant == context;

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
	playerContext->notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_blackwoodCastle, sf::Vector2f(playerContext->GetRect().left + playerContext->GetRect().width / 2.0f, playerContext->GetRect().top - 20.0f), 1.0f);
	notificationWaitingTime = 1.5;
	SelectConfusionTargets();
}


void PlayerStatePrepareAbility::HandleStupidness()
{
	int isStupid = rand() %  3;
	std::cout << "isStupid: " << isStupid << std::endl;
	if (isStupid == 0)
	{
		playerContext->notificationRenderer->AddNotification("Dummheit", g_pFonts->f_blackwoodCastle, sf::Vector2f(playerContext->GetRect().left + playerContext->GetRect().width / 2.0f, playerContext->GetRect().top - 20.0f), 1.0f);
		notificationWaitingTime = 1.5;

		int randomNumber = rand() % 3;

		std::cout << randomNumber << std::endl;

		if (randomNumber == 0)
			SelectRandomAbilityAndTargets();
		else if (randomNumber == 1)
			playerContext->selectedTargets.clear();
		else
		{
			playerContext->actsInConfusion = true;
			SelectConfusionTargets();
		}
	}
	else
		ChangeState();
}


void PlayerStatePrepareAbility::SelectRandomAbilityAndTargets()
{
	playerContext->selectedTargets.clear();
	playerContext->gui->SetCurrentAbility(rand() % 4);

	int numberOfPossibleTargets = 0;
	for (Combatant* c : *(playerContext->allCombatants))
	{
		if (CurrentAbilityCanAimAtCombatant(c))
			numberOfPossibleTargets++;
	}

	if (numberOfPossibleTargets == 0)
		return;

	int selectedTarget = rand() % numberOfPossibleTargets;

	if (numberOfPossibleTargets == 1)
		selectedTarget = 0;

	for (auto c : *playerContext->allCombatants)
	{
		if (CurrentAbilityCanAimAtCombatant(c))
		{
			if (selectedTarget == 0)
			{
				playerContext->selectedTargets.push_back(c);
				SelectAdditionalTargets();
				return;
			}
			selectedTarget--;
		}
	}
}


void PlayerStatePrepareAbility::SelectConfusionTargets()
{
	bool originallyAttackedPlayer = playerContext->selectedTargets[0]->IsPlayer();
	playerContext->selectedTargets.clear();
	if (originallyAttackedPlayer)
		ChooseRandomOpponent();
	else
		ChooseRandomAlly();

	SelectAdditionalConfusionTargets(!originallyAttackedPlayer);
}


bool PlayerStatePrepareAbility::PlayerShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition)
{
	if (!_combatant->IsPlayer() || _combatant == playerContext || _combatant->IsDying())
		return false;

	return CurrentAbilityAttacksAll() && playerContext != _combatant  && playerContext->selectedTargets[0] != _combatant
		|| _combatant->GetBattlePos() < _targetPosition && _combatant->GetBattlePos() > _targetPosition - NumberOfTargetsForCurrentAbility();
}

bool PlayerStatePrepareAbility::EnemyShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition)
{
	if (_combatant->IsPlayer() || _combatant->IsDying())
		return false;
	
	return CurrentAbilityAttacksAllPlayers() && playerContext->selectedTargets[0] != _combatant
		|| CurrentAbilityAttacksAll() && playerContext != _combatant  && playerContext->selectedTargets[0] != _combatant
		|| _combatant->GetBattlePos() > _targetPosition && _combatant->GetBattlePos() < _targetPosition + NumberOfTargetsForCurrentAbility();
}


void PlayerStatePrepareAbility::SelectAdditionalConfusionTargets(bool _selectPlayers)
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

	CombatantState::Render();

	playerContext->RenderTurnMarker();
	RenderAbilityTargetMarker();
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
			playerContext->selectedTargets.push_back(c);
			SelectAdditionalTargets();
			return;
		}
	}
}


void PlayerStatePrepareAbility::SelectAdditionalTargets()
{
	int targetPosition = playerContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* com : (*playerContext->allCombatants))
	{
		if (com == playerContext->selectedTargets[0] || com->IsDying())
			continue;

		if (CurrentAbilityAttacksAllPlayers() && playerContext->selectedTargets[0]->IsPlayer() && com->IsPlayer()
			|| CurrentAbilityAttacksAll() && playerContext != com
			|| com->GetBattlePos() >= targetPosition && com->GetBattlePos() < targetPosition + NumberOfTargetsForCurrentAbility())
			playerContext->selectedTargets.push_back(com);
	}
}


float PlayerStatePrepareAbility::GetAdditionalDamageForCurrentlyAimedCombatant()
{
	int numberOfCombatants = playerContext->selectedTargets.size();
	float additionalDamage = g_pObjectProperties->playerAbilities[playerContext->GetID()][playerContext->gui->GetCurrentAbility()].effectHostile.lessTargetsMoreDamage * (NumberOfTargetsForCurrentAbility() - numberOfCombatants);
	return additionalDamage;
}