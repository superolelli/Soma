#pragma once

#include "../engine/Gui/Button.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "LevelSpecs.hpp"
#include "LevelReward.hpp"

class LevelFinishedPanel
{
public:

	LevelFinishedPanel(CGameEngine *_engine, LevelType _levelType, bool _levelFailed);
	void Update();
	void Render();

	void SetReward(LevelRewards &_rewards);

	bool ContinueButtonClicked() {return continueButtonClicked;}

private:
	CGameEngine *engine;

	CSprite panel;
	CButton continueButton;

	sf::Text levelFinishedText{SfmlCompat::defaultFont()};

	sf::Text rewardFinalDiceText{SfmlCompat::defaultFont()};
	sf::Text rewardFinalCardsText{SfmlCompat::defaultFont()};
	sf::Text rewardDescriptionText{SfmlCompat::defaultFont()};
	sf::Text rewardCardsText{SfmlCompat::defaultFont()};
	sf::Text rewardDiceText{SfmlCompat::defaultFont()};

	sf::Text unlockedItemsText{SfmlCompat::defaultFont()};
	std::vector<CSprite> unlockedItemsSprites;

	bool continueButtonClicked;

	void UpdatePositions();

};