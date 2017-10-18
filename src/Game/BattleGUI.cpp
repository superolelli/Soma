#include "BattleGUI.hpp"



void BattleGUI::Init(CGameEngine *_engine)
{
	engine = _engine;
	int x = 0;

	for (int j = 0; j < 4; j++)
	{
		 x = 150;
		for (int i = 0; i < 4; i++)
		{
			abilities[j][i].Load(g_pTextures->abilities[j][i]);
			abilities[j][i].SetPos(x, 910);
			x += 120;
		}
	}

	currentAbilityFrame.Load(g_pTextures->currentAbilityFrame);
	currentAbilityFrame.SetPos(x - 130, 900);

	currentAbility = 3;
	currentPlayer = Simon;
}



void BattleGUI::Update()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
	{
		for (int i = 0; i < 4; i++)
		{
			if (abilities[currentPlayer][i].GetRect().contains(engine->GetMousePos()))
			{
				currentAbility = i;
				currentAbilityFrame.SetPos(abilities[currentPlayer][i].GetRect().left - 10, abilities[currentPlayer][i].GetRect().top - 10);
			}
		}
	}
}


void BattleGUI::Render()
{
	for (CSprite a : abilities[currentPlayer])
		a.Render(engine->GetWindow());

	currentAbilityFrame.Render(engine->GetWindow());
}
