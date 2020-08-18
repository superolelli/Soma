#include "LevelFinishedPanel.hpp"
#include "Resources\SoundManager.hpp"

void LevelFinishedPanel::Init(CGameEngine *_engine, LevelType _levelType, bool _levelFailed)
{
	engine = _engine;
	levelFailed = _levelFailed;

	continueButton.Load(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Weiter");
	continueButton.SetButtontextCharactersize(30);
	continueButton.SetButtontextColor(sf::Color::White);
	continueButton.SetButtontextFont(g_pFonts->f_blackwoodCastle);
	continueButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	levelFinishedText.setCharacterSize(40);
	levelFinishedText.setFillColor(sf::Color::White);
	levelFinishedText.setOutlineColor(sf::Color::Black);
	levelFinishedText.setOutlineThickness(4);
	levelFinishedText.setFont(g_pFonts->f_kingArthur);

	if (_levelFailed)
	{
		panel.Load(g_pTextures->levelFailedPanel[_levelType]);
		levelFinishedText.setString("Versagt!");
	}
	else
	{
		panel.Load(g_pTextures->levelFinishedPanel[_levelType]);
		levelFinishedText.setString("Level beendet!");
	}

	rewardDiceText.setCharacterSize(20);
	rewardDiceText.setFillColor(sf::Color::White);
	rewardDiceText.setOutlineColor(sf::Color::Black);
	rewardDiceText.setOutlineThickness(2);
	rewardDiceText.setFont(g_pFonts->f_kingArthur);
	rewardDiceText.setString("1x");

	rewardCardsText.setCharacterSize(20);
	rewardCardsText.setFillColor(sf::Color::White);
	rewardCardsText.setOutlineColor(sf::Color::Black);
	rewardCardsText.setOutlineThickness(2);
	rewardCardsText.setFont(g_pFonts->f_kingArthur);
	rewardCardsText.setString("25x");

	PositionComponentsRelativeToPanel();

	continueButtonClicked = false;
}

void LevelFinishedPanel::Update()
{
	if (continueButton.Update(*engine))
		continueButtonClicked = true;
}

void LevelFinishedPanel::Render()
{
	panel.Render(engine->GetRenderTarget());
	engine->GetRenderTarget().draw(levelFinishedText);

	if (!levelFailed) 
	{
		engine->GetRenderTarget().draw(rewardDiceText);
		engine->GetRenderTarget().draw(rewardCardsText);
	}

	continueButton.Render(engine->GetRenderTarget());
}

void LevelFinishedPanel::SetPos(int _x, int _y)
{
	panel.SetPos(_x, _y);
	PositionComponentsRelativeToPanel();
}


void LevelFinishedPanel::SetReward(LevelReward &_reward)
{
	rewardDiceText.setString(std::to_string(_reward.dice) + "x");
	rewardCardsText.setString(std::to_string(_reward.cards) + "x");
}

void LevelFinishedPanel::PositionComponentsRelativeToPanel()
{
	continueButton.SetPos(panel.GetGlobalRect().left + panel.GetGlobalRect().width / 2 - continueButton.GetRect().width / 2, panel.GetGlobalRect().top + 485);

	if(levelFailed)
		levelFinishedText.setPosition(panel.GetGlobalRect().left + panel.GetGlobalRect().width / 2 - levelFinishedText.getGlobalBounds().width / 2, panel.GetGlobalRect().top + 250);
	else
		levelFinishedText.setPosition(panel.GetGlobalRect().left + panel.GetGlobalRect().width / 2 - levelFinishedText.getGlobalBounds().width / 2, panel.GetGlobalRect().top + 175);

	rewardDiceText.setPosition(panel.GetGlobalRect().left + 335, panel.GetGlobalRect().top + 405);
	rewardCardsText.setPosition(panel.GetGlobalRect().left + 545, panel.GetGlobalRect().top + 405);
}
