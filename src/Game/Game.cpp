#include "Game.hpp"
#include "LevelBuilder.hpp"


void Game::Init(CGameEngine * _engine)
{
	m_pGameEngine = _engine;

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	level = LevelBuilder::buildLevel(1);
	adventureGroup.Init(_engine, &notificationRenderer);

	currentGUI = new LevelGUI;
	currentGUI->Init(m_pGameEngine);

	currentBattle = nullptr;
	inBattle = false;
}


void Game::Cleanup()
{
	g_pModels->Quit();  //Muss in letztem Gamestate passieren
	g_pSpritePool->FreeSprites();   //Muss in letztem Gamestate passieren
	adventureGroup.Quit();
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

	if (!g_pVideos->IsPlayingVideo())
	{
		if (inBattle)
			UpdateBattle();
		else
			UpdateLevel();

		notificationRenderer.Update();

		currentGUI->Update();
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

	level->Update(view.getCenter().x);

	if (level->InBattle())
	{
		inBattle = true;
		InitNewBattle();
	}

	adventureGroup.Update(xMove);
}


void Game::UpdateBattle()
{
	currentBattle->Update();

	if (currentBattle->isFinished())
	{
		inBattle = false;
		currentBattle->Quit();
		SAFE_DELETE(currentBattle);

		SAFE_DELETE(currentGUI);
		currentGUI = new LevelGUI;
		currentGUI->Init(m_pGameEngine);
	}

	adventureGroup.Update(0);
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

		currentGUI->Render();
	}

	m_pGameEngine->FlipWindow();
}

