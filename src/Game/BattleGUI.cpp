#include "BattleGUI.hpp"
#include "../Framework/Graphics/RichText.hpp"


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

	for (int i = 0; i < 4; i++)
	{
		if (abilities[currentPlayer][i].GetRect().contains(engine->GetMousePos()))
			ShowTooltip(i);
	}
}



void BattleGUI::ShowTooltip(int _ability)
{
	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(sf::String::fromUtf8(g_pObjectProperties->playerAbilities[currentPlayer][_ability].description.begin(), g_pObjectProperties->playerAbilities[currentPlayer][_ability].description.end()));
	tooltip.setPosition(abilities[currentPlayer][_ability].GetRect().left + 3.0f, abilities[currentPlayer][_ability].GetRect().top - tooltip.getLocalBounds().height - 6.0f);

	sf::FloatRect backgroundRect = tooltip.getLocalBounds();
	sf::RectangleShape background(sf::Vector2f(backgroundRect.width + 6.0f, backgroundRect.height + 9.0f));
	background.setFillColor(sf::Color(0, 0, 0, 200));
	background.setOutlineColor(sf::Color::White);
	background.setPosition(tooltip.getPosition() + sf::Vector2f(-3.0f, -1.0f));

	engine->GetWindow().draw(background);
	engine->GetWindow().draw(tooltip);
}



void BattleGUI::SetCurrentPlayer(int _player)
{
	if (_player >= 0 && _player < 4)
		currentPlayer = (PlayerID)_player;
}
