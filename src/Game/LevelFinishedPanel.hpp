#pragma once

#include "../Framework/Gui/Button.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "LevelSpecs.hpp"

class LevelFinishedPanel
{
public:

	void Init(CGameEngine *_engine, LevelType _levelType, bool _levelFailed);
	void Update();
	void Render();

	void SetPos(int _x, int _y);
	void SetReward(LevelReward &_reward);

	bool ContinueButtonClicked() {return continueButtonClicked;}

private:
	CGameEngine *engine;

	CSprite panel;
	CButton continueButton;
	sf::Text levelFinishedText;
	sf::Text rewardDiceText;
	sf::Text rewardCardsText;

	bool continueButtonClicked;
	bool levelFailed;

	void PositionComponentsRelativeToPanel();

};