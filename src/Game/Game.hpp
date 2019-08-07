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
#include "TreasureStatus.hpp"

class Game : public GameState
{
public:

	void Init(CGameEngine *_engine) override;
	void Cleanup() override;

	void Pause() override;
	void Resume() override;

	void SetTreasureStatusPtr(TreasureStatus *_statusPtr) { treasureStatus = _statusPtr; }

	void HandleEvents() override;
	void Update() override;
	void Render(double _normalizedTimestep) override;

private:

	sf::View view;
	Level *level;
	AdventureGroup adventureGroup;
	GUI *currentGUI;

	TreasureStatus *treasureStatus;

	LevelFinishedPanel levelFinishedPanel;

	Battle *currentBattle;

	NotificationRenderer notificationRenderer;

	bool inBattle;
	bool levelFinished;

	void UpdateLevel();
	void UpdateBattle();
	void InitNewBattle();
};