#include "LevelGUI.hpp"



LevelGUI::LevelGUI(CGameEngine * _engine, LevelStatus *_levelStatus, AdventureGroup *_adventureGroup, LevelType _levelType, NotificationRenderer *_notificationRenderer)
	: engine(_engine)
	, levelType(_levelType)
	, commonGUIParts(_engine, _levelStatus, _notificationRenderer)
	, consumablePanel(_engine, _adventureGroup)
	, levelFinishedPanel(nullptr)
{
}


LevelGUI::~LevelGUI()
{
	SAFE_DELETE(levelFinishedPanel);
}


void LevelGUI::Update()
{
	consumablePanel.Update();
	commonGUIParts.Update();

	if (levelFinishedPanel != nullptr)
	{
		levelFinishedPanel->Update();

		if (levelFinishedPanel->ContinueButtonClicked())
			OnLevelExitedCallback();
	}
}


void LevelGUI::Render()
{
	consumablePanel.Render();
	commonGUIParts.Render();

	if (levelFinishedPanel != nullptr)
		levelFinishedPanel->Render();
}


void LevelGUI::OpenLevelFailedPanel(LevelRewards &_rewards)
{
	levelFinishedPanel = new LevelFinishedPanel(engine, levelType, true);
	levelFinishedPanel->SetReward(_rewards);
}


void LevelGUI::OpenLevelFinishedPanel(LevelRewards &_rewards)
{
	levelFinishedPanel = new LevelFinishedPanel(engine, levelType, false);
	levelFinishedPanel->SetReward(_rewards);
}


void LevelGUI::SetOnLevelExitedCallback(std::function<void(void)> _onLevelExited)
{
	OnLevelExitedCallback = _onLevelExited;
}
