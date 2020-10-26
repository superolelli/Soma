#include "EnemyStatePrepareAbility.hpp"
#include "CombatantStateExecutingAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "Battle.hpp"


EnemyStatePrepareAbility::EnemyStatePrepareAbility(Enemy *_context)
	:CombatantStatePrepareAbility(_context)
{
	enemyContext = _context;
	abilityAnnouncementTime = 1.5;

	ChooseAbility();

	if(enemyContext->actsInConfusion)
		HandleConfusion();
	else
		ChooseTarget();
}


void EnemyStatePrepareAbility::HandleConfusion()
{
	enemyContext->notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_kingArthur, sf::Vector2f(enemyContext->GetRect().left + enemyContext->GetRect().width / 2.0f, enemyContext->GetRect().top - 20.0f), 1.0f);

	bool originallyAttackedPlayer = ChosenAbilityHitsPlayer();

	if (originallyAttackedPlayer)
		ChooseRandomAlly();
	else
		ChooseRandomOpponent();

	SelectAdditionalTargets(!originallyAttackedPlayer);
}


void EnemyStatePrepareAbility::SelectAdditionalTargets(bool _selectPlayers)
{
	if (enemyContext->selectedTargets.empty())
		return;

	int targetPosition = enemyContext->selectedTargets[0]->GetBattlePos();

	for (Combatant* c : (*enemyContext->allCombatants))
	{
		if (_selectPlayers && PlayerShouldBeAddedAsTarget(c, targetPosition) || !_selectPlayers && EnemyShouldBeAddedAsTarget(c, targetPosition))
			enemyContext->selectedTargets.push_back(c);
	}
}

bool EnemyStatePrepareAbility::ChosenAbilityHitsPlayer()
{
    auto positions = g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.position;
    return positions[0] || positions[1] || positions[2] || positions[3];
}

bool EnemyStatePrepareAbility::ChosenAbilityHitsSelf()
{
    return g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.position[enemyContext->battlePosition] || g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackSelf;
}

bool EnemyStatePrepareAbility::PlayerShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition)
{
	if (!_combatant->IsPlayer() || _combatant == enemyContext->selectedTargets[0] || _combatant->IsDying())
		return false;

	if (!enemyContext->actsInConfusion && g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAllPlayers)
		return true;

    if (enemyContext->actsInConfusion && g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAllEnemies)
        return true;

	return g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAll && enemyContext != _combatant
		|| _combatant->GetBattlePos() < _targetPosition && _combatant->GetBattlePos() > _targetPosition - g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.howMany;
}

bool EnemyStatePrepareAbility::EnemyShouldBeAddedAsTarget(Combatant *_combatant, int _targetPosition)
{
	if (_combatant->IsPlayer() || _combatant == enemyContext->selectedTargets[0] || _combatant->IsDying())
		return false;

	if (enemyContext->actsInConfusion && g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAllPlayers)
		return true;

    if (!enemyContext->actsInConfusion && g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAllEnemies)
        return true;

	return enemyContext != _combatant && (g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackAll
		|| _combatant->GetBattlePos() >= _targetPosition && _combatant->GetBattlePos() < _targetPosition + g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.howMany);
}


void EnemyStatePrepareAbility::ChooseAbility()
{
	switch (enemyContext->GetID())
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
	case CombatantID::TequilaJoe:
		ChooseAbilityTequilaJoe();
		break;
	case CombatantID::BillNoface:
		ChooseAbilityBillNoface();
		break;
	case CombatantID::JuanTirador:
		ChooseAbilityJuanTirador();
		break;
	case CombatantID::MarieSaunier:
		ChooseAbilityMarieSaunier();
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


void EnemyStatePrepareAbility::ChooseAbilityTequilaJoe()
{
	bool selfWounded = enemyContext->status.GetCurrentHealth() <= enemyContext->status.GetMaxHealth() - g_pObjectProperties->enemyAbilities[int(enemyAbilities::beer)].effectFriendly.heal;

	bool allyWounded = false;
	for (auto *c : *enemyContext->allCombatants)
	{
		if (!c->IsPlayer() && c->status.GetCurrentHealth() <= c->status.GetMaxHealth() - g_pObjectProperties->enemyAbilities[int(enemyAbilities::spend_a_round)].effectFriendly.heal)
		{
			allyWounded = true;
			break;
		}
	}

	if (selfWounded && allyWounded)
	{
		auto randomNumber = rand() % 3;
		if (randomNumber == 0)
			chosenAbility = enemyAbilities::jabbering;
		else if (randomNumber == 1)
			chosenAbility = enemyAbilities::beer;
		else
			chosenAbility = enemyAbilities::spend_a_round;
	}
	else if (selfWounded)
	{
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::jabbering;
		else
			chosenAbility = enemyAbilities::beer;
	}
	else if (allyWounded)
	{
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::jabbering;
		else
			chosenAbility = enemyAbilities::spend_a_round;
	}
	else
		chosenAbility = enemyAbilities::jabbering;
}


void EnemyStatePrepareAbility::ChooseAbilityBillNoface()
{
	int randomNumber = rand() % 3;
	if (randomNumber == 0)
		chosenAbility = enemyAbilities::shotgun;
	else if (randomNumber == 1)
		chosenAbility = enemyAbilities::panic;
	else
		chosenAbility = enemyAbilities::rattlesnake;
}


void EnemyStatePrepareAbility::ChooseAbilityJuanTirador()
{
	int randomNumber = rand() % 2;
	if (randomNumber == 0)
		chosenAbility = enemyAbilities::duelpistol;
	else
		chosenAbility = enemyAbilities::thug;
}

void EnemyStatePrepareAbility::ChooseAbilityMarieSaunier()
{
	int randomNumber = rand() % 2;
	if (randomNumber == 0)
		chosenAbility = enemyAbilities::distraction;
	else
		chosenAbility = enemyAbilities::spy;
}

bool EnemyStatePrepareAbility::OnlyOneCompanionLeft()
{
	return std::count_if(enemyContext->allCombatants->begin(), enemyContext->allCombatants->end(), [&](Combatant *c) {return !c->IsPlayer(); }) <= 2;
}


void EnemyStatePrepareAbility::ChooseTarget()
{
	if (ChosenAbilityHitsSelf())
		enemyContext->selectedTargets.push_back(enemyContext);
    else if (ChosenAbilityHitsPlayer())
    {
        CheckForMarkedPlayers();
        if (enemyContext->selectedTargets.empty())
            ChooseRandomPlayer();
    }
	else 
        ChooseRandomEnemy();
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
				SelectAdditionalTargets(true);
				return;
			}
		}
	}
}


bool EnemyStatePrepareAbility::CanAimAtCombatant(Combatant *_combatant)
{
	return g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.position[_combatant->GetBattlePos()] && !_combatant->IsDying();
}


void EnemyStatePrepareAbility::ChooseRandomEnemy()
{
    ChooseRandomAlly();
    SelectAdditionalTargets(false);

    if(enemyContext->selectedTargets.empty())
    {
        CombatantStateIdle *newState = new CombatantStateIdle(enemyContext);
        enemyContext->ChangeState(newState);
    }
}

void EnemyStatePrepareAbility::ChooseRandomPlayer()
{
	int numberOfPlayers = std::accumulate((*enemyContext->allCombatants).begin(), (*enemyContext->allCombatants).end(), 0, [&](int sum, Combatant* c) {if (c->IsPlayer() && CanAimAtCombatant(c)) return sum + 1; else return sum; });

	if (numberOfPlayers > 0)
	{
		int target = rand() % numberOfPlayers;

		if (numberOfPlayers == 1)
			target = 0;

		for (Combatant* c : (*enemyContext->allCombatants))
		{
			if (c->IsPlayer() && CanAimAtCombatant(c))
			{
				if (target == 0)
				{
					enemyContext->selectedTargets.push_back(c);
					SelectAdditionalTargets(ChosenAbilityHitsPlayer());
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



void EnemyStatePrepareAbility::Update()
{
	//Cannot be done in constructor since it does a state change
	if (enemyContext->selectedTargets.empty())
	{
		CombatantStateIdle *newState = new CombatantStateIdle(enemyContext);
		enemyContext->ChangeState(newState);
		return;
	}

	abilityAnnouncementTime -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();

	if (abilityAnnouncementTime <= 0.0)
	{
		CombatantStateExecutingAbility *newState = new CombatantStateExecutingAbility(enemyContext, &g_pObjectProperties->enemyAbilities[int(chosenAbility)]);
		enemyContext->ChangeState(newState);
	}
}


void EnemyStatePrepareAbility::Render()
{
	enemyContext->RenderShadow();

	enemyContext->combatantObject->setTimeElapsed(g_pTimer->GetElapsedTimeAsMilliseconds());
	enemyContext->combatantObject->render();
	enemyContext->combatantObject->playSoundTriggers();

	if (abilityAnnouncementTime > 0.0 && !enemyContext->selectedTargets.empty())
		RenderAbilityAnnouncement();

	enemyContext->RenderTurnMarker();

	if (abilityAnnouncementTime >= 0.0)
		RenderAbilityTargetMarker();

	enemyContext->statusBar.Render();
}



void EnemyStatePrepareAbility::RenderAbilityAnnouncement()
{
	g_pSpritePool->abilityAnnouncementBanner.SetPos(enemyContext->engine->GetRenderTarget().getView().getCenter().x + 100.0f, 150.0f);
	g_pSpritePool->abilityAnnouncementBanner.ChangeString(0, GetChosenAbilityName());
	g_pSpritePool->abilityAnnouncementBanner.SetTextPosCentered(0);
	g_pSpritePool->abilityAnnouncementBanner.MoveText(0, 0, -20);
	g_pSpritePool->abilityAnnouncementBanner.Render(enemyContext->engine->GetRenderTarget());
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