#include "AdventureGroup.hpp"




void AdventureGroup::Init(CGameEngine *_engine, NotificationRenderer *_notificationRenderer)
{
	adventurer[CombatantID::Ole] = new PlayerOle;
	adventurer[CombatantID::Ole]->Init(CombatantID::Ole, _engine, _notificationRenderer);

	adventurer[CombatantID::Anna] = new PlayerAnna;
	adventurer[CombatantID::Anna]->Init(CombatantID::Anna, _engine, _notificationRenderer);

	adventurer[CombatantID::Simon] = new PlayerSimon;
	adventurer[CombatantID::Simon]->Init(CombatantID::Simon, _engine, _notificationRenderer);

	adventurer[CombatantID::Markus] = new PlayerMarkus;
	adventurer[CombatantID::Markus]->Init(CombatantID::Markus, _engine, _notificationRenderer);


	int x = GROUP_OFFSET_LEFT;
	for (auto a : adventurer)
	{
		a->SetPos(x, GROUP_Y_POS);
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
		if(a->Status().GetCurrentHealth() > 0)
			a->Render();
	}
}

