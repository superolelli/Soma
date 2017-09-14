#include "Game.hpp"



void Game::Init(CGameEngine * _engine)
{
	m_pGameEngine = _engine;

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	level.Init();
	adventureGroup.Init();
}


void Game::Cleanup()
{
	adventureGroup.Quit();
	m_pGameEngine = nullptr;
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
	int xMove = 0;

	if(m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
		m_pGameEngine->StopEngine();

	if (m_pGameEngine->GetKeystates(KeyID::Left) == Keystates::Held)
		xMove = -2;

	if (m_pGameEngine->GetKeystates(KeyID::Right) == Keystates::Held)
		xMove = 2;

	view.move(xMove, 0);
	adventureGroup.Update(xMove);
}



void Game::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);

	m_pGameEngine->GetWindow().setView(view);

	level.Render(m_pGameEngine->GetWindow(), view.getCenter().x - view.getSize().x / 2);
	adventureGroup.Render();

	m_pGameEngine->FlipWindow();
}

