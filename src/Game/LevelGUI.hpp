#pragma once

#include "GUI.hpp"
#include "ConsumablePanel.hpp"
#include "ResourcesStatusBar.hpp"
#include "LevelFinishedPanel.hpp"
#include "LevelGUICommon.hpp"
#include "LevelReward.hpp"

class LevelGUI : public GUI
{
public:

	~LevelGUI();

	virtual void Init(CGameEngine *_engine, GameStatus *_gameStatus, LevelStatus *_levelStatus, AdventureGroup *_adventureGroup, LevelType _levelType, NotificationRenderer *_notificationRenderer);
	virtual void Update() override;
	virtual void Render() override;

	void OpenLevelFailedPanel(LevelRewards &_rewards);
	void OpenLevelFinishedPanel(LevelRewards &_rewards);

	void SetOnLevelExitedCallback(std::function<void(void)> _onLevelExited);

private:
	CGameEngine *engine;
	GameStatus *gameStatus;

	LevelType levelType;

	LevelGUICommon commonGUIParts;

	ConsumablePanel consumablePanel;
	LevelFinishedPanel *levelFinishedPanel;

	std::function<void(void)> OnLevelExitedCallback;
};