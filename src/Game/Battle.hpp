#pragma once
#include "Resources\Resources.hpp"
#include "AdventureGroup.hpp"
#include "BattleGUI.hpp"
#include "PlayerEnum.hpp"
#include "Abilities.hpp"

enum abilityPhase {ready, aimed, finished};

class Battle
{
public:

	void Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine);
	void Update();
	void Render();
	void Quit();

	bool battleFinished() { return isBattleFinished; }

private:

	SpriterEngine::EntityInstance *enemy[4];
	AdventureGroup *players;
	BattleGUI *gui;
	CGameEngine *engine;

	bool isBattleFinished;

	int currentPlayer;
	abilityPhase abilityStatus;

	bool AimChosen();
	bool CompetitantClicked(int _id);
};