#include "EnemyStatePrepareAbility.hpp"
#include "EnemyStateExecutingAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "Battle.hpp"


EnemyStatePrepareAbility::EnemyStatePrepareAbility(Enemy *_context)
	:EnemyState(_context)
{
	abilityAnnouncementTime = 1.5f;

	ChooseAbility();

	if (enemyContext->actsInConfusion)
	{
		enemyContext->notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_kingArthur, sf::Vector2f(enemyContext->GetRect().left - enemyContext->GetRect().width / 2.0f, enemyContext->GetRect().top - 20.0f), 1.0f);
		enemyContext->ChooseRandomAlly();

		if (enemyContext->selectedTargets.empty())
		{
			CombatantStateIdle *newState = new CombatantStateIdle(enemyContext);
			enemyContext->ChangeState(newState);
		}
		else
			SelectAdditionalEnemies();
	}
	else
	{
		ChooseTarget();
	}
}


void EnemyStatePrepareAbility::ChooseAbility()
{
	switch (enemyContext->enemyID)
	{
	case CombatantID::Gesetzloser:
		chosenAbility = enemyAbilities::springfield;
		break;
	case CombatantID::Indianer:
		chosenAbility = enemyAbilities::tomahawk;
		break;
	case CombatantID::Abtruenniger:
		ChooseAbilityAbtruenniger();
		break;
	case CombatantID::Greg:
		ChooseAbilityGreg();
		break;
	case CombatantID::Apachekid:
		ChooseAbilityApacheKid();
		break;
	case CombatantID::Hilfssheriff:
		ChooseAbilityHilfssherrif();
		break;
	case CombatantID::BigSpencer:
		ChooseAbilityBigSpencer();
		break;
	}
}

void EnemyStatePrepareAbility::ChooseAbilityGreg()
{
	if (enemyContext->battle->EnemyDiedLastRound() && enemyContext->status.GetCurrentHealth() < enemyContext->status.GetMaxHealth())
		chosenAbility = enemyAbilities::bury_the_dead;
	else
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::shovel_hit;
		else
			chosenAbility = enemyAbilities::gravedigger_gaze;
}

void EnemyStatePrepareAbility::ChooseAbilityBigSpencer()
{
	if (rand() % 2 == 0)
		chosenAbility = enemyAbilities::punch;
	else
		chosenAbility = enemyAbilities::brawl;
}

void EnemyStatePrepareAbility::ChooseAbilityAbtruenniger()
{
	if (rand() % 2 == 0)
		chosenAbility = enemyAbilities::bang;
	else
		chosenAbility = enemyAbilities::fanning;
}

void EnemyStatePrepareAbility::ChooseAbilityApacheKid()
{
	if (OnlyOneCompanionLeft())
		chosenAbility = enemyAbilities::indians;
	else
		chosenAbility = enemyAbilities::knife;
}


void EnemyStatePrepareAbility::ChooseAbilityHilfssherrif()
{
	for (auto *c : *enemyContext->allCombatants)
	{
		if (c->IsPlayer() && c->Status().IsMarked())
		{
			chosenAbility = enemyAbilities::bang;
			return;
		}
	}

	if (rand() % 2 == 0)
		chosenAbility = enemyAbilities::bang;
	else
		chosenAbility = enemyAbilities::bounty;
}


bool EnemyStatePrepareAbility::OnlyOneCompanionLeft()
{
	return std::count_if(enemyContext->allCombatants->begin(), enemyContext->allCombatants->end(), [&](Combatant *c) {return !c->IsPlayer(); }) <= 2;
}


void EnemyStatePrepareAbility::ChooseTarget()
{
	enemyContext->selectedTargets.clear();

	if (chosenAbility == enemyAbilities::bury_the_dead || chosenAbility == enemyAbilities::indians)
		enemyContext->selectedTargets.push_back(enemyContext);
	else {
		CheckForMarkedPlayers();

		if (enemyContext->selectedTargets.empty())
		{
			ChooseRandomPlayer();
		}
	}
}

void EnemyStatePrepareAbility::CheckForMarkedPlayers()
{
	for (Combatant* c : (*enemyContext->allCombatants))
	{
		if (c->IsPlayer() && c->Status().IsMarked() && CanAimAtCombatant(c))
		{
			if (rand() % 10 < 8)
			{
				enemyContext->selectedTargets.push_back(c);
				return;
			}
		}
	}
}


bool EnemyStatePrepareAbility::CanAimAtCombatant(Combatant *_combatant)
{
	return g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.position[_combatant->GetBattlePos()] && !_combatant->IsDying();
}



void EnemyStatePrepareAbility::ChooseRandomPlayer()
{
	int numberOfPlayers = std::accumulate((*enemyContext->allCombatants).begin(), (*enemyContext->allCombatants).end(), 0, [&](int sum, Combatant* c) {if (c->IsPlayer() && CanAimAtCombatant(c)) return sum + 1; else return sum; });

	if (numberOfPlayers > 0)
	{
		int target = rand() % numberOfPlayers;
		for (Combatant* c : (*enemyContext->allCombatants))
		{
			if (c->IsPlayer() && CanAimAtCombatant(c))
			{
				if (target == 0)
				{
					enemyContext->selectedTargets.push_back(c);
					SelectAdditionalPlayers();
					return;
				}
				target--;
			}
		}
	}
	else
	{
		CombatantStateIdle *newState = new CombatantStateIdle(enemyContext);
		enemyContext->ChangeState(newState);
	}
}


void EnemyStatePrepareAbility::SelectAdditionalPlayers()
{
	int targetPosition = enemyContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*enemyContext->allCombatants))
	{
		if (g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAllPlayers)
		{
			if (enemyContext->selectedTargets[0] != c && c->IsPlayer())
				enemyContext->selectedTargets.push_back(c);
		}
		else if (g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAll && enemyContext != c  && enemyContext->selectedTargets[0] != c
			|| c->GetBattlePos() < targetPosition && c->GetBattlePos() > targetPosition - g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.howMany)
			enemyContext->selectedTargets.push_back(c);
	}
}


void EnemyStatePrepareAbility::SelectAdditionalEnemies()
{
	int targetPosition = enemyContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*enemyContext->allCombatants))
	{
		if (c == enemyContext)
			continue;

		if (enemyContext->actsInConfusion && g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAllPlayers)
		{
			if (enemyContext->selectedTargets[0] != c && !c->IsPlayer())
				enemyContext->selectedTargets.push_back(c);
		}
		else if (g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAll && enemyContext != c  && enemyContext->selectedTargets[0] != c || c->GetBattlePos() > targetPosition && c->GetBattlePos() < targetPosition + g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.howMany)
			enemyContext->selectedTargets.push_back(c);
	}
}



void EnemyStatePrepareAbility::Update()
{
	abilityAnnouncementTime -= g_pTimer->GetElapsedTime().asSeconds();

	if (abilityAnnouncementTime <= 0.0f)
	{
		EnemyStateExecutingAbility *newState = new EnemyStateExecutingAbility(enemyContext, chosenAbility);
		enemyContext->ChangeState(newState);
	}
}


void EnemyStatePrepareAbility::Render()
{
	enemyContext->RenderShadow();

	enemyContext->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
	enemyContext->combatantObject->render();
	enemyContext->combatantObject->playSoundTriggers();

	if (abilityAnnouncementTime > 0.0f)
		RenderAbilityAnnouncement();

	enemyContext->RenderTurnMarker();

	if (abilityAnnouncementTime >= 0.0f)
		RenderAbilityTargetMarker();

	enemyContext->statusBar.Render();
}



void EnemyStatePrepareAbility::RenderAbilityAnnouncement()
{
	g_pSpritePool->abilityAnnouncementBanner.SetPos(enemyContext->engine->GetWindow().getView().getCenter().x + 100.0f, 150.0f);
	g_pSpritePool->abilityAnnouncementBanner.ChangeString(0, GetChosenAbilityName());
	g_pSpritePool->abilityAnnouncementBanner.SetTextPosCentered(0);
	g_pSpritePool->abilityAnnouncementBanner.MoveText(0, 0, -20);
	g_pSpritePool->abilityAnnouncementBanner.Render(enemyContext->engine->GetWindow());
}


sf::String &EnemyStatePrepareAbility::GetChosenAbilityName()
{
	return g_pObjectProperties->enemyAbilities[int(chosenAbility)].name;
}

void EnemyStatePrepareAbility::RenderAbilityTargetMarker()
{
	for (Combatant *c : enemyContext->selectedTargets)
		c->RenderAbilityTargetMarker();
}