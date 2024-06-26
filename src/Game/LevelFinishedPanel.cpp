#include "LevelFinishedPanel.hpp"
#include "Resources\SoundManager.hpp"

LevelFinishedPanel::LevelFinishedPanel(CGameEngine *_engine, LevelType _levelType, bool _levelFailed)
	: engine(_engine)
	, panel(g_pTextures->levelFinishedPanel[_levelType])
	, continueButton(g_pTextures->levelFinishedButton, Buttontypes::Motion_Up, "Weiter")
	, continueButtonClicked(false)
{
	continueButton.SetButtontextCharactersize(40);
	continueButton.SetButtontextColor(sf::Color::White);
	continueButton.SetButtontextFont(g_pFonts->f_blackwoodCastle);
	continueButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	levelFinishedText.setCharacterSize(50);
	levelFinishedText.setFillColor(sf::Color::White);
	levelFinishedText.setOutlineColor(sf::Color::Black);
	levelFinishedText.setOutlineThickness(5);
	levelFinishedText.setFont(g_pFonts->f_kingArthur);

	if (_levelFailed)
		levelFinishedText.setString("Versagt!");
	else
		levelFinishedText.setString("Level beendet!");

	unlockedItemsText.setCharacterSize(41);
	unlockedItemsText.setFillColor(sf::Color::White);
	unlockedItemsText.setOutlineColor(sf::Color::Black);
	unlockedItemsText.setOutlineThickness(4);
	unlockedItemsText.setFont(g_pFonts->f_blackwoodCastle);

	rewardFinalDiceText.setCharacterSize(40);
	rewardFinalDiceText.setFillColor(sf::Color::White);
	rewardFinalDiceText.setOutlineColor(sf::Color::Black);
	rewardFinalDiceText.setOutlineThickness(4);
	rewardFinalDiceText.setFont(g_pFonts->f_kingArthur);

	rewardFinalCardsText.setCharacterSize(40);
	rewardFinalCardsText.setFillColor(sf::Color::White);
	rewardFinalCardsText.setOutlineColor(sf::Color::Black);
	rewardFinalCardsText.setOutlineThickness(4);
	rewardFinalCardsText.setFont(g_pFonts->f_kingArthur);

	rewardDescriptionText.setCharacterSize(32);
	rewardDescriptionText.setFillColor(sf::Color::White);
	rewardDescriptionText.setOutlineColor(sf::Color::Black);
	rewardDescriptionText.setOutlineThickness(3);
	rewardDescriptionText.setFont(g_pFonts->f_blackwoodCastle);

	rewardCardsText.setCharacterSize(32);
	rewardCardsText.setFillColor(sf::Color::White);
	rewardCardsText.setOutlineColor(sf::Color::Black);
	rewardCardsText.setOutlineThickness(3);
	rewardCardsText.setFont(g_pFonts->f_blackwoodCastle);

	rewardDiceText.setCharacterSize(32);
	rewardDiceText.setFillColor(sf::Color::White);
	rewardDiceText.setOutlineColor(sf::Color::Black);
	rewardDiceText.setOutlineThickness(3);
	rewardDiceText.setFont(g_pFonts->f_blackwoodCastle);

	UpdatePositions();
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
	engine->GetRenderTarget().draw(unlockedItemsText);
	engine->GetRenderTarget().draw(rewardDescriptionText);
	engine->GetRenderTarget().draw(rewardCardsText);
	engine->GetRenderTarget().draw(rewardDiceText);
	engine->GetRenderTarget().draw(rewardFinalDiceText);
	engine->GetRenderTarget().draw(rewardFinalCardsText);

	for (auto& unlockedItems : unlockedItemsSprites)
		unlockedItems.Render(engine->GetRenderTarget());
	
	continueButton.Render(engine->GetRenderTarget());
}

void LevelFinishedPanel::SetReward(LevelRewards &_rewards)
{
	int finalDice = 0;
	int finalCards = 0;

	std::string descriptionString;
	std::string cardsString;
	std::string diceString;

	for (auto& [cards, dice, description] : _rewards.achievements)
	{
		finalCards += cards;
		finalDice += dice;

		descriptionString += description + "\n";
		cardsString += std::to_string(cards) + "\n";
		diceString += std::to_string(dice) + "\n";
	}

	if(!_rewards.unlockedItems.empty())
		unlockedItemsText.setString("Items freigeschaltet!");

	unlockedItemsSprites.clear();
	for (int i = 0; i < _rewards.unlockedItems.size(); i++)
	{
		unlockedItemsSprites.emplace_back(g_pTextures->item[_rewards.unlockedItems[i]]);
		unlockedItemsSprites.back().SetPos(static_cast<float>(1445 + (i % 2) * 150), static_cast<float>(300 + (i / 2) * 115));
	}

	rewardDescriptionText.setString(descriptionString);
	rewardCardsText.setString(cardsString);
	rewardDiceText.setString(diceString);

	rewardFinalDiceText.setString(std::to_string(finalDice) + "x");
	rewardFinalCardsText.setString(std::to_string(finalCards) + "x");
}

void LevelFinishedPanel::UpdatePositions()
{
	continueButton.SetPos(1390, 850);
	levelFinishedText.setPosition(330, 230);
	unlockedItemsText.setPosition(1410, 220);
	rewardDescriptionText.setPosition(330, 360);
	rewardCardsText.setPosition(1070, 360);
	rewardDiceText.setPosition(1260, 360);
	rewardFinalCardsText.setPosition(430, 880);
	rewardFinalDiceText.setPosition(920, 880);
}
