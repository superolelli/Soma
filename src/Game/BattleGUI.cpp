#include "BattleGUI.hpp"



void BattleGUI::Init(CGameEngine *_engine)
{
	engine = _engine;

	int x = 150;
	for (int i = 0; i < 4; i++)
	{
		abilities[i].Load(g_pTextures->abilities[i]);
		abilities[i].SetPos(x, 910);
		x += 120;
	}

	currentAbilityFrame.Load(g_pTextures->currentAbilityFrame);
	currentAbilityFrame.SetPos(x - 130, 900);

	currentAbility = 3;
}



void BattleGUI::Update()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
	{
		for (int i = 0; i < 4; i++)
		{
			if (abilities[i].GetRect().contains(engine->GetMousePos()))
			{
				currentAbility = i;
				currentAbilityFrame.SetPos(abilities[i].GetRect().left - 10, abilities[i].GetRect().top - 10);
			}
		}
	}
}


void BattleGUI::Render()
{
	for (CSprite a : abilities)
		a.Render(engine->GetWindow());

	currentAbilityFrame.Render(engine->GetWindow());
}
