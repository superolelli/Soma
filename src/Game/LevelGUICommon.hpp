#pragma once

#include "GUI.hpp"
#include "ResourcesStatusBar.hpp"
#include "../Framework/Gui/Bar.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"
#include "LevelStatus.hpp"

class AdventureGroup;
class GameStatus;

class LevelGUICommon
{
public:

	void Init(CGameEngine* _engine, GameStatus *_gameStatus, LevelStatus *_levelStatus, NotificationRenderer *_notificationRenderer);
	void Update();
	void Render();

private:
	CGameEngine* engine;
	GameStatus* gameStatus;
	NotificationRenderer* notificationRenderer;

	Bar fatigueBar;
	ResourcesStatusBar resourcesStatusBar;
};