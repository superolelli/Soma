#pragma once

#include "GameStatus.hpp"
#include "../Framework/Gui/Button.hpp"

class SkillPanel
{
public:

	void Init(GameStatus *_gameStatus, CGameEngine *_engine);
	void Update();
	void Render();

	void Open();

private:
	bool closed;

	GameStatus *gameStatus;
	CGameEngine *engine;

	int currentPlayer;
	int currentAbility;

	sf::IntRect abilityPanelRect[4];

	CSprite skillPanel;
	CSprite bridgePiece;
	CSprite skilledIndicator;
	CSprite abilityPlaceholders;
	CSprite connectionsNotSkilled[6];
	CSprite connectionsSkilled[6];

	CSprite abilities[4][4];
	CSprite skills[4][4][8];

	sf::Text currentPlayerName;
	sf::Text panelTitle;

	CButton buttonNext;
	CButton buttonPrevious;
	CButton buttonClose;

	void RenderConnection(int connection, int parentSkill);
	void RenderSkilledIndicators();
	void ShowTooltip(int _skill);
};