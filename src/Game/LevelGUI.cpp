#include "LevelGUI.hpp"



void LevelGUI::Init(CGameEngine * _engine, GameStatus *_gameStatus, AdventureGroup *_adventureGroup)
{
	engine = _engine;
	gameStatus = _gameStatus;

	consumablePanel.Init(engine, _gameStatus, _adventureGroup);
	resourcesStatusBar.Init(engine);
	levelFinishedPanel = nullptr;
}


LevelGUI::~LevelGUI()
{
	consumablePanel.Quit();
	resourcesStatusBar.Quit();
	SAFE_DELETE(levelFinishedPanel);
}


void LevelGUI::Update()
{
	consumablePanel.Update();
	resourcesStatusBar.Update(gameStatus->GetCardsAmount(), gameStatus->GetDiceAmount());

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
	resourcesStatusBar.Render();

	if (levelFinishedPanel != nullptr)
		levelFinishedPanel->Render();
}


void LevelGUI::OpenLevelFailedPanel()
{
	levelFinishedPanel = new LevelFinishedPanel;
	levelFinishedPanel->Init(engine, true);
	levelFinishedPanel->SetPos(engine->GetWindowSize().x / 2 - 458, 190);
}


void LevelGUI::OpenLevelFinishedPanel(LevelReward &_reward)
{
	levelFinishedPanel = new LevelFinishedPanel;
	levelFinishedPanel->Init(engine, false);
	levelFinishedPanel->SetReward(_reward);
	levelFinishedPanel->SetPos(engine->GetWindowSize().x / 2 - 458, 200);
}


void LevelGUI::SetOnLevelExitedCallback(std::function<void(void)> _onLevelExited)
{
	OnLevelExitedCallback = _onLevelExited;
}
