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
#include "SpeechBubbleManager.hpp"
#include "LevelReward.hpp"

class Game : public GameState
{
public:

	Game(CGameEngine *_engine, LevelType _levelType);
	~Game();

	void Pause() override;
	void Resume() override;

	void HandleEvents() override;
	void Update() override;
	void Render(double _normalizedTimestep) override;

	void SetOnGameFinishedCallback(std::function<void(void)> _onGameFinished);

private:

	LevelType levelType;
	LevelRewards levelRewards;

	sf::View view;
	Level *level;
	AdventureGroup adventureGroup;
	GUI *currentGUI;

	Battle *currentBattle;

	NotificationRenderer notificationRenderer;
	DialogManager dialogManager;
	SpeechBubbleManager speechBubbleManager;

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

	void HandlePlayerFatigue();

	void HandleBattleIntro();

	void HandleFinishedLevel();
	void OpenBattleRewardDialog();
};