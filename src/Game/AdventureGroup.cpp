#include "AdventureGroup.hpp"




void AdventureGroup::Init(CGameEngine *_engine, NotificationRenderer *_notificationRenderer, GameStatus *_gameStatus)
{
	adventurer[CombatantID::Ole] = new PlayerOle(CombatantID::Ole, _engine, _notificationRenderer);
	adventurer[CombatantID::Ole]->Init();

	adventurer[CombatantID::Anna] = new PlayerAnna(CombatantID::Anna, _engine, _notificationRenderer);
	adventurer[CombatantID::Anna]->Init();

	adventurer[CombatantID::Simon] = new PlayerSimon(CombatantID::Simon, _engine, _notificationRenderer);
	adventurer[CombatantID::Simon]->Init();

	adventurer[CombatantID::Markus] = new PlayerMarkus(CombatantID::Markus, _engine, _notificationRenderer);
	adventurer[CombatantID::Markus]->Init();


	int x = GROUP_OFFSET_LEFT;
	for (int i = 0; i < 4; i++)
	{
		adventurer[i]->SetPos(x, GROUP_Y_POS);
		adventurer[i]->SetEquipment(_gameStatus->GetEquipmentStats(i));
		adventurer[i]->SetEquipment(_gameStatus->GetDiamondStats(i));
		x += PLAYER_SPACING;
	}
}


void AdventureGroup::Quit()
{
	for (auto &a : adventurer)
	{
		a->Quit();
		SAFE_DELETE(a);
	}
}

void AdventureGroup::Update(int xMove)
{
	for (auto a : adventurer)
	{
		a->Update(xMove, xMove != 0);
	}
	
}


void AdventureGroup::Render()
{
	for (auto a : adventurer)
	{
		if(a->Status().GetAttribute("currentHealth") > 0)
			a->Render();
	}
}

void AdventureGroup::SetFatigueLevelOfAllPlayers(CombatantStatus::FatigueLevel _level)
{
	for (auto& a : adventurer)
		a->Status().SetFatigueLevel(_level);
}


bool AdventureGroup::IsDead()
{
	for (auto &a : adventurer)
	{
		if (a->Status().GetAttribute("currentHealth") > 0)
			return false;
	}

	return true;
}
