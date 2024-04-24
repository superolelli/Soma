#include "LevelFinishedPanel.hpp"
#include "Resources\SoundManager.hpp"

void LevelFinishedPanel::Init(CGameEngine *_engine, LevelType _levelType, bool _levelFailed)
{
	engine = _engine;

	panel.Load(g_pTextures->levelFinishedPanel[_levelType]);

	continueButton.Load(g_pTextures->levelFinishedButton, Buttontypes::Motion_Up, "Weiter");
	continueButton.SetButtontextCharactersize(35);
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

	unlockedItemsText.setCharacterSize(40);
	unlockedItemsText.setFillColor(sf::Color::White);
	unlockedItemsText.setOutlineColor(sf::Color::Black);
	unlockedItemsText.setOutlineThickness(4);
	unlockedItemsText.setFont(g_pFonts->f_kingArthur);

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
	engine->GetRenderTarget().draw(unlockedItemsText);
	engine->GetRenderTarget().draw(rewardDescriptionText);
	engine->GetRenderTarget().draw(rewardCardsText);
	engine->GetRenderTarget().draw(rewardDiceText);
	engine->GetRenderTarget().draw(rewardFinalDiceText);
	engine->GetRenderTarget().draw(rewardFinalCardsText);
	
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

	// TODO: Unlocked items

	rewardDescriptionText.setString(descriptionString);
	rewardCardsText.setString(cardsString);
	rewardDiceText.setString(diceString);

	rewardFinalDiceText.setString(std::to_string(finalDice) + "x");
	rewardFinalCardsText.setString(std::to_string(finalCards) + "x");

	// TODO
	// unlockedItemsText.setString("Items freigeschaltet!");
}

void LevelFinishedPanel::UpdatePositions()
{
	continueButton.SetPos(1390, 850);
	levelFinishedText.setPosition(330, 230);
	unlockedItemsText.setPosition(1410, 220);
	rewardDescriptionText.setPosition(330, 360);
	rewardCardsText.setPosition(1050, 360);
	rewardDiceText.setPosition(1250, 360);
	rewardFinalCardsText.setPosition(410, 880);
	rewardFinalDiceText.setPosition(920, 880);
}
