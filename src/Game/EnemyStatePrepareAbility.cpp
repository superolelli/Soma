#include "EnemyStatePrepareAbility.hpp"
#include "CombatantStateExecutingAbility.hpp"
#include "CombatantStateIdle.hpp"
#include "Battle.hpp"


EnemyStatePrepareAbility::EnemyStatePrepareAbility(Enemy *_context)
	: CombatantStatePrepareAbility(_context)
	, enemyContext(_context)
	, abilityAnnouncementTime(1.5)
{
	ChooseAbility();

	if(enemyContext->actsInConfusion)
		HandleConfusion();
	else
		ChooseTarget();
}


void EnemyStatePrepareAbility::HandleConfusion()
{
	enemyContext->notificationRenderer->AddNotification("Verwirrt!", g_pFonts->f_blackwoodCastle, sf::Vector2f(enemyContext->GetRect().left + enemyContext->GetRect().width / 2.0f, enemyContext->GetRect().top - 20.0f), 1.0f);

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
    return g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.attackSelf;
}

bool EnemyStatePrepareAbility::ChosenAbilityHitsOnlySelf()
{
	auto hitPositions = g_pObjectProperties->enemyAbilities[int(chosenAbility)].possibleAims.position;
	return ChosenAbilityHitsSelf() && !hitPositions[4] && !hitPositions[5] && !hitPositions[6] && !hitPositions[7];
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
		ChooseAbilityGesetzloser();
		break;
	case CombatantID::Indianer:
		ChooseAbilityIndianer();
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
	case CombatantID::BruderZacharias:
		ChooseAbilityBruderZacharias();
		break;
	case CombatantID::TheodoraKrayenborg:
		ChooseAbilityTheodoraKrayenborg();
		break;
	case CombatantID::BasiliusKartov:
		ChooseAbilityBasiliusKartov();
		break;
	}
}

void EnemyStatePrepareAbility::ChooseAbilityGreg()
{
	if (enemyContext->battle->EnemyDiedLastRound() && enemyContext->status.GetAttribute("currentHealth") < enemyContext->status.GetAttribute("maxHealth"))
		chosenAbility = enemyAbilities::bury_the_dead;
	else
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::shovel_hit;
		else
			chosenAbility = enemyAbilities::gravedigger_gaze;
}

void EnemyStatePrepareAbility::ChooseAbilityGesetzloser()
{

	if (context->Status().NumberOfMisses() >= 2)
		chosenAbility = enemyAbilities::springfield;
	else if (rand() % 2 == 0)
		chosenAbility = enemyAbilities::springfield;
	else
		chosenAbility = enemyAbilities::miss;
}

void EnemyStatePrepareAbility::ChooseAbilityIndianer()
{
	bool playerAwake = false;
	for (auto c : *context->allCombatants)
	{
		if (c->IsPlayer() && !c->Status().IsAsleep())
		{
			playerAwake = true;
			break;
		}
	}

	if(!playerAwake)
		chosenAbility = enemyAbilities::tomahawk;
	else
	{
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::knock_out;
		else
			chosenAbility = enemyAbilities::tomahawk;
	}
}

void EnemyStatePrepareAbility::ChooseAbilityBigSpencer()
{
	int numberOfPlayerBuffs = 0;
	bool dynamitePossible = true;
	for (auto c : *context->allCombatants)
	{
		if (c->IsPlayer() && c->Status().IsBuffed())
			numberOfPlayerBuffs++;
		if (c->Status().HasDynamite())
			dynamitePossible = false;
	}

	if (numberOfPlayerBuffs > 0)
	{
		if (dynamitePossible)
		{
			if (rand() % 2 == 0)
				chosenAbility = enemyAbilities::brawl;
			else
				chosenAbility = enemyAbilities::dynamite;
		}
		else
			chosenAbility = enemyAbilities::brawl;
	}
	else if (dynamitePossible)
	{
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::punch;
		else
			chosenAbility = enemyAbilities::dynamite;
	}
	else
		chosenAbility = enemyAbilities::punch;
}


void EnemyStatePrepareAbility::ChooseAbilityAbtruenniger()
{
	bool canHitFanning = false;
	for (auto& c : *context->allCombatants)
		canHitFanning |= c->GetBattlePos() > 0 && c->IsPlayer();

	if (rand() % 2 == 0 || !canHitFanning)
		chosenAbility = enemyAbilities::bang;
	else
		chosenAbility = enemyAbilities::fanning;
}

void EnemyStatePrepareAbility::ChooseAbilityApacheKid()
{
	if (OnlyOneCompanionLeft())
		chosenAbility = enemyAbilities::indians;
	else if (rand() % 2 == 0)
		chosenAbility = enemyAbilities::knife;
	else
		chosenAbility = enemyAbilities::no_pain;
}


void EnemyStatePrepareAbility::ChooseAbilityHilfssherrif()
{
	bool otherAlly = false;
	bool unmarkedPlayer = false;
	for (auto *c : *enemyContext->allCombatants)
	{
		if (!c->IsPlayer() && c->GetID() != CombatantID::Hilfssheriff)
			otherAlly = true;

		if (c->IsPlayer() && !c->Status().HasBounty())
			unmarkedPlayer = true;
	}

	if (!otherAlly)
		chosenAbility = enemyAbilities::bang;
	else if (unmarkedPlayer)
	{
		if (rand() % 2 == 0)
			chosenAbility = enemyAbilities::deputy;
		else
			chosenAbility = enemyAbilities::bounty;
	}
	else
		chosenAbility = enemyAbilities::deputy;
}


void EnemyStatePrepareAbility::ChooseAbilityTequilaJoe()
{
	bool selfWounded = enemyContext->status.GetAttribute("currentHealth") <= enemyContext->status.GetAttribute("maxHealth") - g_pObjectProperties->enemyAbilities[int(enemyAbilities::beer)].effectFriendly.heal;

	bool allyWounded = false;
	for (auto *c : *enemyContext->allCombatants)
	{
		if (!c->IsPlayer() && c->status.GetAttribute("currentHealth") <= c->status.GetAttribute("maxHealth") - g_pObjectProperties->enemyAbilities[int(enemyAbilities::spend_a_round)].effectFriendly.heal)
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


void EnemyStatePrepareAbility::ChooseAbilityBruderZacharias()
{
	bool allyWounded = false;
	for (auto* c : *enemyContext->allCombatants)
	{
		if (!c->IsPlayer() && c->status.GetAttribute("currentHealth") <= c->status.GetAttribute("maxHealth") - g_pObjectProperties->enemyAbilities[int(enemyAbilities::doctor)].effectFriendly.heal)
		{
			allyWounded = true;
			break;
		}
	}

	if (allyWounded)
	{
		int randomNumber = rand() % 5;
		if (randomNumber == 0)
			chosenAbility = enemyAbilities::staff;
		else
			chosenAbility = enemyAbilities::doctor;
	}
	else
		chosenAbility = enemyAbilities::staff;
}

void EnemyStatePrepareAbility::ChooseAbilityTheodoraKrayenborg()
{
	bool enemyBuffed = false;
	for (auto* c : *enemyContext->allCombatants)
	{
		if (c->IsPlayer() && c->Status().IsBuffed())
		{
			enemyBuffed = true;
			break;
		}
	}

	if (enemyBuffed)
	{
		int randomNumber = rand() % 6;
		if (randomNumber < 4)
			chosenAbility = enemyAbilities::crystal_ball;
		else if (randomNumber == 4)
			chosenAbility = enemyAbilities::throwing_knives;
		else
			chosenAbility = enemyAbilities::psychic;
	}
	else
	{
		int randomNumber = rand() % 2;
		if (randomNumber == 0)
			chosenAbility = enemyAbilities::psychic;
		else
			chosenAbility = enemyAbilities::throwing_knives;
	}
}


void EnemyStatePrepareAbility::ChooseAbilityBasiliusKartov()
{
	int randomNumber = rand() % 3;
	switch (randomNumber) {
	case 0:
		chosenAbility = enemyAbilities::whip;
		break;
	case 1:
		chosenAbility = enemyAbilities::gloves;
		break;
	case 2:
		chosenAbility = enemyAbilities::grandmaster;
		break;
	}
}


bool EnemyStatePrepareAbility::OnlyOneCompanionLeft()
{
	return std::count_if(enemyContext->allCombatants->begin(), enemyContext->allCombatants->end(), [&](Combatant *c) {return !c->IsPlayer(); }) <= 2;
}


void EnemyStatePrepareAbility::ChooseTarget()
{
	if (chosenAbility == enemyAbilities::doctor)
	{
		ChooseTargetForDoctor();
		return;
	}
	else if (chosenAbility == enemyAbilities::knock_out)
	{
		ChooseTargetIf([](Combatant* c) {return c->IsPlayer() && !c->Status().IsAsleep();});
		return;
	}
	else if (chosenAbility == enemyAbilities::bounty)
	{
		ChooseTargetIf([](Combatant *c) {return c->IsPlayer() && !c->Status().HasBounty();});
		return;
	}
	else if (chosenAbility == enemyAbilities::crystal_ball)
	{
		ChooseTargetIf([](Combatant *c) {return c->IsPlayer() && c->Status().IsBuffed();});
		return;
	}

	if (ChosenAbilityHitsOnlySelf())
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

void EnemyStatePrepareAbility::ChooseTargetIf(std::function<bool(Combatant*)> predicate)
{
	int numberOfTargets = 0;

	for (auto c : *context->allCombatants)
	{
		if (predicate(c))
			numberOfTargets++;
	}

	int target = rand() % numberOfTargets;

	if (numberOfTargets == 1)
		target = 0;

	for (auto c : *context->allCombatants)
	{
		if (predicate(c))
		{
			if (target == 0)
			{
				enemyContext->selectedTargets.push_back(c);
				return;
			}

			target--;
		}
	}
}

void EnemyStatePrepareAbility::ChooseTargetForDoctor()
{
	for (auto* c : *enemyContext->allCombatants)
	{
		if (!c->IsPlayer() && !c->IsDying() && c->status.GetAttribute("currentHealth") <= c->status.GetAttribute("maxHealth") - g_pObjectProperties->enemyAbilities[int(enemyAbilities::doctor)].effectFriendly.heal)
		{
			enemyContext->selectedTargets.push_back(c);
			break;
		}
	}

	if (enemyContext->selectedTargets.empty())
	{
		chosenAbility = enemyAbilities::staff;
		CheckForMarkedPlayers();
		if (enemyContext->selectedTargets.empty())
			ChooseRandomPlayer();
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
    ChooseRandomAlly(ChosenAbilityHitsSelf());
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

	CombatantState::Render();

	if (abilityAnnouncementTime > 0.0 && !enemyContext->selectedTargets.empty())
		RenderAbilityAnnouncement();

	enemyContext->RenderTurnMarker();

	if (abilityAnnouncementTime >= 0.0)
		RenderAbilityTargetMarker();
}



void EnemyStatePrepareAbility::RenderAbilityAnnouncement()
{
	g_pSpritePool->abilityAnnouncementBanner->SetPos(enemyContext->engine->GetRenderTarget().getView().getCenter().x + 100.0f, 150.0f);
	g_pSpritePool->abilityAnnouncementBanner->ChangeString(0, GetChosenAbilityName());
	g_pSpritePool->abilityAnnouncementBanner->SetTextPosCentered(0);
	g_pSpritePool->abilityAnnouncementBanner->MoveText(0, 0, -20);
	g_pSpritePool->abilityAnnouncementBanner->Render(enemyContext->engine->GetRenderTarget());
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