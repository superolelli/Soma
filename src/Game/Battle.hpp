#pragma once
#include "Resources\Resources.hpp"
#include "AdventureGroup.hpp"
#include "BattleGUI.hpp"



class Battle
{
public:

	void Init(int _xView, AdventureGroup *_adventureGroup);
	void Update();
	void Render();
	void Quit();

	bool battleFinished() { return isBattleFinished; }

private:

	SpriterEngine::EntityInstance *enemy[4];
	AdventureGroup *players;
	BattleGUI *gui;

	bool isBattleFinished;
};