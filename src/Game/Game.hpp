#pragma once

#include "../Framework/Gamestate.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include "../Game/Resources/Resources.hpp"
#include "Level.hpp"
#include "AdventureGroup.hpp"
#include "Battle.hpp"
#include "GUI.hpp"
#include "BattleGUI.hpp"
#include "LevelGUI.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"
#include "LevelFinishedPanel.hpp"
#include "GameStatus.hpp"
#include "DialogManager.hpp"

class Game : public GameState
{
public:

	void SetGameStatusPtr(GameStatus *_statusPtr) { gameStatus = _statusPtr; } //has to be called before Game::Init()

	void Init(CGameEngine *_engine) override;
	void Cleanup() override;

	void Pause() override;
	void Resume() override;

	void HandleEvents() override;
	void Update() override;
	void Render(double _normalizedTimestep) override;

private:

	sf::View view;
	Level *level;
	AdventureGroup adventureGroup;
	GUI *currentGUI;

	GameStatus *gameStatus;

	LevelFinishedPanel levelFinishedPanel;

	Battle *currentBattle;

	NotificationRenderer notificationRenderer;
	DialogManager dialogManager;

	bool inBattle;
	bool levelFinished;

	void UpdateLevel();
	void UpdateBattle();
	void InitNewBattle();
};