#include "TreasureStatus.hpp"

void TreasureStatus::Init(CGameEngine * _engine)
{
	engine = _engine;

	statusBar.Load(g_pTextures->statusBar);
	statusBar.SetPos(engine->GetWindowSize().x - statusBar.GetGlobalRect().width, 0);

	diceAmountText.setCharacterSize(14);
	diceAmountText.setFillColor(sf::Color::White);
	//diceAmountText.setOutlineColor(sf::Color::Black);
	//diceAmountText.setOutlineThickness(2);
	diceAmountText.setFont(g_pFonts->f_kingArthur);
	diceAmountText.setString("0");

	cardsAmountText.setCharacterSize(14);
	cardsAmountText.setFillColor(sf::Color::White);
	//cardsAmountText.setOutlineColor(sf::Color::Black);
	//cardsAmountText.setOutlineThickness(2);
	cardsAmountText.setFont(g_pFonts->f_kingArthur);
	cardsAmountText.setString("0");

	PositionComponentsRelativeToPanel();

	dice = 0;
	cards = 0;
}

void TreasureStatus::RenderStatusBar()
{
	statusBar.Render(engine->GetWindow());
	engine->GetWindow().draw(diceAmountText);
	engine->GetWindow().draw(cardsAmountText);
}

void TreasureStatus::SetPos(int _x, int _y)
{
	statusBar.SetPos(_x, _y);
	PositionComponentsRelativeToPanel();
}

int TreasureStatus::GetDiceAmount()
{
	return dice;
}

int TreasureStatus::GetCardsAmount()
{
	return cards;
}

void TreasureStatus::AddDice(int _amount)
{
	dice += _amount;
	diceAmountText.setString(std::to_string(dice));
}

void TreasureStatus::AddCards(int _amount)
{
	cards += _amount;
	cardsAmountText.setString(std::to_string(cards));
}

void TreasureStatus::RemoveDice(int _amount)
{
	dice = std::max(0, dice - _amount);
	diceAmountText.setString(std::to_string(dice));
}

void TreasureStatus::RemoveCards(int _amount)
{
	cards = std::max(0, cards - _amount);
	cardsAmountText.setString(std::to_string(cards));
}

void TreasureStatus::PositionComponentsRelativeToPanel()
{
	diceAmountText.setPosition(statusBar.GetGlobalRect().left + 80, statusBar.GetGlobalRect().top + 20);
	cardsAmountText.setPosition(statusBar.GetGlobalRect().left + 150, statusBar.GetGlobalRect().top + 20);
}
