#pragma once

#include "GUI.hpp"
#include "ResourcesStatusBar.hpp"
#include "../Framework/Gui/Bar.hpp"

class AdventureGroup;
class GameStatus;

class LevelGUICommon
{
public:

	void Init(CGameEngine* _engine, GameStatus *_gameStatus);
	void Update();
	void Render();

private:
	CGameEngine* engine;
	GameStatus* gameStatus;

	Bar fatigueBar;
	ResourcesStatusBar resourcesStatusBar;
};