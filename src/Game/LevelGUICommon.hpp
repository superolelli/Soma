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

	LevelGUICommon(CGameEngine* _engine, LevelStatus *_levelStatus, NotificationRenderer *_notificationRenderer);
	void Update();
	void Render();

private:
	CGameEngine* engine;
	NotificationRenderer* notificationRenderer;

	Bar fatigueBar;
	ResourcesStatusBar resourcesStatusBar;
};