#include "Game.hpp"
#include "LevelBuilder.hpp"


void Game::Init(CGameEngine * _engine)
{
	m_pGameEngine = _engine;

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	level = LevelBuilder::buildLevel(gameStatus->bangLevel, &dialogManager, gameStatus);
	adventureGroup.Init(_engine, &notificationRenderer, gameStatus);

	consumablePanel.Init(m_pGameEngine, gameStatus, &adventureGroup);

	resourcesStatusBar.Init(m_pGameEngine);

	LevelGUI *newGui = new LevelGUI;
	newGui->Init(m_pGameEngine);
	currentGUI = newGui;

	currentBattle = nullptr;
	inBattle = false;
	levelFinished = false;

	g_pMusic->SetCurrentEnvironment(MusicEnvironment::bangEnvironment);
	g_pMusic->PlayMusic();
}


void Game::Cleanup()
{
	resourcesStatusBar.Quit();
	consumablePanel.Quit();
	adventureGroup.Quit();
	dialogManager.Quit();
	m_pGameEngine = nullptr;
	SAFE_DELETE(currentGUI);
	SAFE_DELETE(level);
}


void Game::Pause()
{
}


void Game::Resume()
{
}


void Game::HandleEvents()
{
	m_pGameEngine->ProcessEvents();
}


void Game::Update()
{
	if(m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
		m_pGameEngine->StopEngine();

	m_pGameEngine->GetWindow().setView(view);

	dialogManager.Update();
	g_pMusic->Update();
	g_pSounds->Update();

	if (dialogManager.IsDialogOpen())
		return;

	if (!g_pVideos->IsPlayingVideo())
	{
		if (inBattle)
			UpdateBattle();
		else
		{
			UpdateLevel();
			consumablePanel.Update();
		}

		notificationRenderer.Update();

		currentGUI->Update();
		resourcesStatusBar.Update(gameStatus->GetCardsAmount(), gameStatus->GetDiceAmount());
	}

	if (levelFinished)
	{
		levelFinishedPanel.Update();
		if (levelFinishedPanel.ContinueButtonClicked())
		{
			if (!adventureGroup.IsDead())
			{
				gameStatus->AddDice(level->GetReward().dice);
				gameStatus->AddCards(level->GetReward().cards);
				gameStatus->bangLevel++;
			}
			g_pMusic->SetCurrentEnvironment(MusicEnvironment::mainRoomEnvironment);
			m_pGameEngine->PopState();
		}
	}

	g_pVideos->Update();
}


void Game::UpdateLevel()
{
	int xMove = 0;

	if (m_pGameEngine->GetKeystates(KeyID::Left) == Keystates::Held)
		xMove = -2;

	if (m_pGameEngine->GetKeystates(KeyID::Right) == Keystates::Held)
		xMove = 2;

	if(!level->IsAtEnd(view.getCenter().x))
		view.move(xMove, 0);

	level->Update(view.getCenter().x, m_pGameEngine);

	if (level->InBattle())
	{
		inBattle = true;
		InitNewBattle();
	}

	adventureGroup.Update(xMove);
}


void Game::UpdateBattle()
{
	adventureGroup.Update(0);
	currentBattle->Update();

	if (currentBattle->isFinished())
	{
		if (adventureGroup.IsDead())
		{
			levelFinished = true;
			levelFinishedPanel.Init(m_pGameEngine, true);
			levelFinishedPanel.SetPos(m_pGameEngine->GetWindowSize().x / 2 - 458, 190);
		}
		else if (currentBattle->isBossBattle)
		{
			levelFinished = true;

			levelFinishedPanel.Init(m_pGameEngine, false);
			levelFinishedPanel.SetReward(level->GetReward());
			levelFinishedPanel.SetPos(m_pGameEngine->GetWindowSize().x / 2 - 458, 200);
		}

		inBattle = false;
		currentBattle->Quit();
		SAFE_DELETE(currentBattle);

		SAFE_DELETE(currentGUI);
		LevelGUI *newGui = new LevelGUI;
		newGui->Init(m_pGameEngine);
		currentGUI = newGui;

		g_pMusic->SetBattleEnded();
	}
}


void Game::InitNewBattle()
{
	SAFE_DELETE(currentGUI);
	BattleGUI *newGui = new BattleGUI;
	newGui->Init(m_pGameEngine);
	newGui->SetAdventureGroup(&adventureGroup);
	currentGUI = newGui;

	currentBattle = new Battle;
	currentBattle->Init(view.getCenter().x, &adventureGroup, (BattleGUI*)currentGUI, m_pGameEngine, &notificationRenderer, level->GetEnemyIDs(), level->IsBossBattle());
}




void Game::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);

	if (g_pVideos->IsPlayingVideo())
	{
		m_pGameEngine->GetWindow().setView(m_pGameEngine->GetWindow().getDefaultView());
		g_pVideos->Render(m_pGameEngine->GetWindow());
	}
	else 
	{
		m_pGameEngine->GetWindow().setView(view);
		level->Render(m_pGameEngine->GetWindow(), view.getCenter().x - view.getSize().x / 2);

		if (currentBattle == nullptr)
			adventureGroup.Render();
		else
			currentBattle->Render();

		notificationRenderer.Render(m_pGameEngine->GetWindow());

		m_pGameEngine->GetWindow().setView(m_pGameEngine->GetWindow().getDefaultView());

		if (currentBattle != nullptr)
			currentBattle->RenderAbilityAnimations();
		else
			consumablePanel.Render();

		currentGUI->Render();
		resourcesStatusBar.Render();

		 if (levelFinished)
			levelFinishedPanel.Render();

		 dialogManager.RenderDialogs();
	}

	m_pGameEngine->FlipWindow();
}

