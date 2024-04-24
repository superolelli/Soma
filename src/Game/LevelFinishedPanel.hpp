#pragma once

#include "../Framework/Gui/Button.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "LevelSpecs.hpp"
#include "LevelReward.hpp"

class LevelFinishedPanel
{
public:

	void Init(CGameEngine *_engine, LevelType _levelType, bool _levelFailed);
	void Update();
	void Render();

	void SetReward(LevelRewards &_rewards);

	bool ContinueButtonClicked() {return continueButtonClicked;}

private:
	CGameEngine *engine;

	CSprite panel;
	CButton continueButton;
	sf::Text levelFinishedText;

	sf::Text unlockedItemsText;

	sf::Text rewardFinalDiceText;
	sf::Text rewardFinalCardsText;

	sf::Text rewardDescriptionText;
	sf::Text rewardCardsText;
	sf::Text rewardDiceText;

	bool continueButtonClicked;

	void UpdatePositions();

};