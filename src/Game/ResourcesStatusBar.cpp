#include "ResourcesStatusBar.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"

void ResourcesStatusBar::Init(CGameEngine *_engine)
{
	engine = _engine;
	statusBar.Load(g_pTextures->statusBar);
	statusBar.SetPos(1732, 0);

	diceAmountText.setCharacterSize(14);
	diceAmountText.setFillColor(sf::Color::White);
	diceAmountText.setFont(g_pFonts->f_kingArthur);

	cardsAmountText.setCharacterSize(14);
	cardsAmountText.setFillColor(sf::Color::White);
	cardsAmountText.setFont(g_pFonts->f_kingArthur);

	PositionComponentsRelativeToPanel();
}

void ResourcesStatusBar::Quit()
{
}

void ResourcesStatusBar::Update(int _cards, int _dice)
{
	diceAmountText.setString(std::to_string(_dice));
	cardsAmountText.setString(std::to_string(_cards));
}

void ResourcesStatusBar::Render()
{
	statusBar.Render(engine->GetWindow());
	engine->GetWindow().draw(diceAmountText);
	engine->GetWindow().draw(cardsAmountText);
}

void ResourcesStatusBar::SetPos(int _x, int _y)
{
	statusBar.SetPos(_x, _y);
	PositionComponentsRelativeToPanel();
}

void ResourcesStatusBar::PositionComponentsRelativeToPanel()
{
	diceAmountText.setPosition(statusBar.GetGlobalRect().left + 80, statusBar.GetGlobalRect().top + 20);
	cardsAmountText.setPosition(statusBar.GetGlobalRect().left + 150, statusBar.GetGlobalRect().top + 20);
}
