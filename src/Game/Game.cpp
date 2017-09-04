#include "Game.hpp"



void Game::Init(CGameEngine * _engine)
{
	m_pGameEngine = _engine;

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	level.Init();
}


void Game::Cleanup()
{

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
	if(m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
		m_pGameEngine->StopEngine();

	if (m_pGameEngine->GetKeystates(KeyID::Left) == Keystates::Held)
		view.move(-5, 0);

	if (m_pGameEngine->GetKeystates(KeyID::Right) == Keystates::Held)
		view.move(5, 0);
}



void Game::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);

	m_pGameEngine->GetWindow().setView(view);

	level.Render(m_pGameEngine->GetWindow(), view.getCenter().x - view.getSize().x / 2);

	m_pGameEngine->FlipWindow();
}

