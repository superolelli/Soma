#pragma once

#include "GUI.hpp"
#include "ConsumablePanel.hpp"
#include "ResourcesStatusBar.hpp"
#include "LevelFinishedPanel.hpp"


class LevelGUI : public GUI
{
public:

	~LevelGUI();

	virtual void Init(CGameEngine *_engine, GameStatus *_gameStatus, AdventureGroup *_adventureGroup);
	virtual void Update() override;
	virtual void Render() override;

	void OpenLevelFailedPanel();
	void OpenLevelFinishedPanel(LevelReward &_reward);

	void SetOnLevelExitedCallback(std::function<void(void)> _onLevelExited);

private:
	CGameEngine *engine;
	GameStatus *gameStatus;

	ConsumablePanel consumablePanel;
	ResourcesStatusBar resourcesStatusBar;
	LevelFinishedPanel *levelFinishedPanel;

	std::function<void(void)> OnLevelExitedCallback;
};