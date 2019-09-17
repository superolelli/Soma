#include "GameStatus.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void GameStatus::Init(CGameEngine * _engine)
{
	engine = _engine;

	dice = 20;
	cards = 0;

	statusBar.Load(g_pTextures->statusBar);
	statusBar.SetPos(engine->GetWindowSize().x - statusBar.GetGlobalRect().width, 0);

	diceAmountText.setCharacterSize(14);
	diceAmountText.setFillColor(sf::Color::White);
	//diceAmountText.setOutlineColor(sf::Color::Black);
	//diceAmountText.setOutlineThickness(2);
	diceAmountText.setFont(g_pFonts->f_kingArthur);
	diceAmountText.setString(std::to_string(dice));

	cardsAmountText.setCharacterSize(14);
	cardsAmountText.setFillColor(sf::Color::White);
	//cardsAmountText.setOutlineColor(sf::Color::Black);
	//cardsAmountText.setOutlineThickness(2);
	cardsAmountText.setFont(g_pFonts->f_kingArthur);
	cardsAmountText.setString(std::to_string(cards));

	PositionComponentsRelativeToPanel();

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int a = 0; a < 8; a++)
				skillAcquired[i][j][a] = false;
		}
	}

	bangLevel = 1;
	kutschfahrtLevel = 1;
	tichuLevel = 1;
}

void GameStatus::RenderStatusBar()
{
	statusBar.Render(engine->GetWindow());
	engine->GetWindow().draw(diceAmountText);
	engine->GetWindow().draw(cardsAmountText);
}

void GameStatus::SetPos(int _x, int _y)
{
	statusBar.SetPos(_x, _y);
	PositionComponentsRelativeToPanel();
}

bool GameStatus::IsSkillAcquired(int player, int ability, int skill)
{
	return skillAcquired[player][ability][skill];
}

void GameStatus::AcquireSkill(int player, int ability, int skill)
{
	if (!skillAcquired[player][ability][skill])
	{
		skillAcquired[player][ability][skill] = true;
		g_pObjectProperties->playerAbilities[player][ability].applySkill(g_pObjectProperties->skills[player][ability][skill]);
	}
}

int GameStatus::GetDiceAmount()
{
	return dice;
}

int GameStatus::GetCardsAmount()
{
	return cards;
}

void GameStatus::AddDice(int _amount)
{
	dice += _amount;
	diceAmountText.setString(std::to_string(dice));
}

void GameStatus::AddCards(int _amount)
{
	cards += _amount;
	cardsAmountText.setString(std::to_string(cards));
}

void GameStatus::RemoveDice(int _amount)
{
	dice = std::max(0, dice - _amount);
	diceAmountText.setString(std::to_string(dice));
}

void GameStatus::RemoveCards(int _amount)
{
	cards = std::max(0, cards - _amount);
	cardsAmountText.setString(std::to_string(cards));
}

void GameStatus::PositionComponentsRelativeToPanel()
{
	diceAmountText.setPosition(statusBar.GetGlobalRect().left + 80, statusBar.GetGlobalRect().top + 20);
	cardsAmountText.setPosition(statusBar.GetGlobalRect().left + 150, statusBar.GetGlobalRect().top + 20);
}