#include "AdventureGroup.hpp"




void AdventureGroup::Init(CGameEngine *_engine, NotificationRenderer *_notificationRenderer)
{
	adventurer[PlayerID::Ole] = new PlayerOle;
	adventurer[PlayerID::Ole]->Init(PlayerID::Ole, _engine, _notificationRenderer);

	adventurer[PlayerID::Anna] = new PlayerAnna;
	adventurer[PlayerID::Anna]->Init(PlayerID::Anna, _engine, _notificationRenderer);

	adventurer[PlayerID::Simon] = new PlayerSimon;
	adventurer[PlayerID::Simon]->Init(PlayerID::Simon, _engine, _notificationRenderer);

	adventurer[PlayerID::Markus] = new PlayerMarkus;
	adventurer[PlayerID::Markus]->Init(PlayerID::Markus, _engine, _notificationRenderer);


	int x = GROUP_OFFSET_LEFT;
	for (auto a : adventurer)
	{
		a->SetPos(x, GROUP_Y_POS);
		x += PLAYER_SPACING;
	}
}


void AdventureGroup::Quit()
{
	for (auto a : adventurer)
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
		a->Render();
}

void AdventureGroup::RenderWithHealthBars(sf::RenderTarget &_target)
{
	for (auto a : adventurer)
	{
		a->Render();
		a->RenderHealthBar(_target);
	}
}

