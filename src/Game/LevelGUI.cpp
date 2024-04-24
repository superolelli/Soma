#include "LevelGUI.hpp"



void LevelGUI::Init(CGameEngine * _engine, GameStatus *_gameStatus, LevelStatus *_levelStatus, AdventureGroup *_adventureGroup, LevelType _levelType, NotificationRenderer *_notificationRenderer)
{
	engine = _engine;
	gameStatus = _gameStatus;

	levelType = _levelType;

	commonGUIParts.Init(_engine, _gameStatus, _levelStatus, _notificationRenderer);
	consumablePanel.Init(engine, _gameStatus, _adventureGroup);

	levelFinishedPanel = nullptr;
}


LevelGUI::~LevelGUI()
{
	consumablePanel.Quit();
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
	levelFinishedPanel = new LevelFinishedPanel;
	levelFinishedPanel->Init(engine, levelType, true);
	levelFinishedPanel->SetReward(_rewards);
}


void LevelGUI::OpenLevelFinishedPanel(LevelRewards &_rewards)
{
	levelFinishedPanel = new LevelFinishedPanel;
	levelFinishedPanel->Init(engine, levelType, false);
	levelFinishedPanel->SetReward(_rewards);
}


void LevelGUI::SetOnLevelExitedCallback(std::function<void(void)> _onLevelExited)
{
	OnLevelExitedCallback = _onLevelExited;
}
