#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"
#include "AbilityTooltip.hpp"

class SkillPanel
{
public:

	void Init(GameStatus *_gameStatus, CGameEngine *_engine);
	void Update();
	void Render();

	void Open(int _player = 0);
	bool IsOpen() { return !closed; }

private:
	bool closed;

	GameStatus *gameStatus;
	CGameEngine *engine;

	int currentPlayer;
	int currentAbility;
	int currentSkill;

	sf::IntRect abilityPanelRect[4];

	CSprite skillPanel;
	CSprite bridgePiece;
	CSprite skilledIndicator;
	CSprite abilityPlaceholders;
	CSprite connectionsNotSkilled[6];
	CSprite connectionsSkilled[6];
	CSprite diceSymbol;

	CSprite abilities[4][4];
	CSprite skills[4][4][8];
	CSprite currentSkillFrame;

	int skillCost[8] = {1, 3, 5, 1, 3, 5, 7, 8};

	sf::Text currentPlayerName;
	sf::Text panelTitle;
	sf::Text abilityName[4];
	sf::Text chosenSkillName;
	sf::Text chosenSkillPrice;

	AbilityTooltip abilityTooltip;

	CButton buttonNext;
	CButton buttonPrevious;
	CButton buttonClose;
	CButton buttonBuy;

	void RenderConnection(int connection, int parentSkill);
	void RenderSkilledIndicators();
	void ShowTooltip(int _skill);

	void ShowAbilityTooltip(sf::IntRect &_abilityRect);

	void UpdateGUIForChosenSkill();
	void UpdateGUIForChosenPlayer();

	void UpdateChosenSkillName();
	void UpdateAbilityNames();
	void UpdateCurrentSkillFrame();
	void UpdateBuyButton();

	void CheckButtonsForPlayerChoosing();
	void CheckBuyButton();

	void RecolorSkills();

	bool SkillCanBeAcquired(int _player, int _ability, int _skill);
};