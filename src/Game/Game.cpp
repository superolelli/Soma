#include "Game.hpp"



void Game::Init(CGameEngine * _engine)
{
	m_pGameEngine = _engine;

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	level.Init();
	adventureGroup.Init();

	currentGUI = new LevelGUI;
	currentGUI->Init(m_pGameEngine);

	currentBattle = nullptr;
	inBattle = false;
}


void Game::Cleanup()
{
	adventureGroup.Quit();
	g_pModels->Quit();  //Muss in letztem Gamestate passieren
	m_pGameEngine = nullptr;
	SAFE_DELETE(currentGUI);
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


	if (inBattle)
		UpdateBattle();
	else
		UpdateLevel();

	currentGUI->Update();
}


void Game::UpdateLevel()
{
	int xMove = 0;

	if (m_pGameEngine->GetKeystates(KeyID::Left) == Keystates::Held)
		xMove = -2;

	if (m_pGameEngine->GetKeystates(KeyID::Right) == Keystates::Held)
		xMove = 2;

	view.move(xMove, 0);
	level.Update(view.getCenter().x);

	if (level.InBattle())
	{
		inBattle = true;
		InitNewBattle();
	}

	adventureGroup.Update(xMove);
}


void Game::UpdateBattle()
{
	currentBattle->Update();

	if (currentBattle->battleFinished())
	{
		inBattle = false;
		currentBattle->Quit();
		SAFE_DELETE(currentBattle);
	}

	adventureGroup.Update(0);
}


void Game::InitNewBattle()
{
	SAFE_DELETE(currentGUI);
	currentGUI = new BattleGUI;
	currentGUI->Init(m_pGameEngine);

	currentBattle = new Battle;
	currentBattle->Init(view.getCenter().x, &adventureGroup);
}




void Game::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);

	m_pGameEngine->GetWindow().setView(view);

	level.Render(m_pGameEngine->GetWindow(), view.getCenter().x - view.getSize().x / 2);
	adventureGroup.Render();
	
	if (currentBattle != nullptr)
		currentBattle->Render();

	m_pGameEngine->GetWindow().setView(m_pGameEngine->GetWindow().getDefaultView());

	currentGUI->Render();

	m_pGameEngine->FlipWindow();
}

