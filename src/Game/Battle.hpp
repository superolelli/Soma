#pragma once
#include "Resources\Resources.hpp"



class Battle
{
public:
	void Init(int _xView);
	void Update();
	void Render();
	void Quit();

	bool battleFinished() { return isBattleFinished; }

private:

	SpriterEngine::EntityInstance *enemyObject[4];

	bool isBattleFinished;
};