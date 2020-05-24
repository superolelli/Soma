#pragma once

#include "../Framework/Gamestate.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include "../Game/Resources/Resources.hpp"
#include "Level.hpp"
#include "AdventureGroup.hpp"
#include "Battle.hpp"
#include "GUI.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"
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

	void SetOnGameFinishedCallback(std::function<void(void)> _onGameFinished);

private:

	sf::View view;
	Level *level;
	AdventureGroup adventureGroup;
	GUI *currentGUI;
	GameStatus *gameStatus;

	Battle *currentBattle;

	NotificationRenderer notificationRenderer;
	DialogManager dialogManager;

	sf::Shader blurShader;

	bool inBattle;
	bool isPlayingBattleIntro;
	double afterIntroWaitingTime;
	bool levelFinished;

	std::function<void(void)> OnGameFinished;

	void UpdateLevel();
	void UpdateBattle();
	void InitNewBattle();
	void InitLevelGUI();

	void HandleBattleIntro();

	void HandleFinishedLevel();
	void OpenBattleRewardDialog();
};