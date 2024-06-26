#include "Gameengine.hpp"
#include "Gamestate.hpp"


CGameEngine::CGameEngine(std::string const &_name)
	: currentRenderTexture(0)
	, nextGameState(nullptr)
	, nextAction(action::hold)
	, nextActionCount(0)
	, m_running(true)
	, m_simpleRenderLoop(false)
{
	m_Window.Init(_name);
	m_CursorManager.Init(&m_Window);
	m_RenderTextures[0].create(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
	m_RenderTextures[1].create(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
}



CGameEngine::~CGameEngine()
{
	ClearStates();
	SAFE_DELETE(nextGameState);
	m_Window.Quit();
}



void CGameEngine::Run()
{
	double lag = 0.0;

	//the game loop
	while (m_running)
	{

		//update the timer and calculate the lag
		g_pTimer->Update();

		if (m_simpleRenderLoop)
		{
			m_pStates.back()->HandleEvents();
			m_pStates.back()->Update();
			m_CursorManager.Update();
		}
		else
		{
			lag += g_pTimer->GetElapsedTimeAsMilliseconds();
			//updates
			while (lag >= MS_PER_UPDATE)
			{
				m_pStates.back()->HandleEvents();
				m_pStates.back()->Update();
				m_CursorManager.Update();
				//checks if the current state needs to be popped
				CheckStates();
				lag -= MS_PER_UPDATE;
				g_pTimer->ResetTimeSinceLastUpdate();
			}
		}

		//renders
		m_pStates.back()->Render(lag / (double)MS_PER_UPDATE);
	}
}


void CGameEngine::PushState(GameState * _state)
{
	nextGameState = _state;
	nextAction = action::push;
}


void CGameEngine::PushStateImmediately(GameState * _state)
{
	m_pStates.push_back(_state);
}



void CGameEngine::PopState()
{
	nextAction = action::pop;
	nextActionCount++;
}

void CGameEngine::PopStateImmediately()
{
	SAFE_DELETE(m_pStates.back());
	m_pStates.pop_back();
}

void CGameEngine::FlushRenderTarget(sf::Shader *_shader)
{
	m_RenderTextures[currentRenderTexture].display();
	sf::Sprite sprite;
	sprite.setTexture(m_RenderTextures[currentRenderTexture].getTexture());
	m_Window.GetRenderWindow()->draw(sprite, _shader);
}

void CGameEngine::ApplyShaderToRenderTarget(sf::Shader* _shader)
{
	m_RenderTextures[currentRenderTexture].display();
	sf::Sprite sprite;
	sprite.setTexture(m_RenderTextures[currentRenderTexture].getTexture());
	currentRenderTexture = !currentRenderTexture;
	m_RenderTextures[currentRenderTexture].draw(sprite, _shader);
}


void CGameEngine::ChangeState(GameState * _state)
{
	nextGameState = _state;
	nextAction = action::change;
}

void CGameEngine::ChangeStateImmediately(GameState * _state)
{
	ClearStates();
	m_pStates.push_back(_state);
}



void CGameEngine::ClearStates()
{
	//delete every state on the stack
	for (auto it = m_pStates.rbegin(); it != m_pStates.rend(); it++)
	{
		SAFE_DELETE(*it);
	}

	m_pStates.clear();
}


void CGameEngine::CheckStates()
{
	switch (nextAction)
	{
	case action::push:
		PushStateImmediately(nextGameState);
		break;

	case action::pop:
		for (int i = 0; i < nextActionCount; i++)
		{
			SAFE_DELETE(m_pStates.back());
			m_pStates.pop_back();
		}
		break;

	case action::change:
		ChangeStateImmediately(nextGameState);
		break;
	}
	
	nextGameState = nullptr;
	nextAction = action::hold;
	nextActionCount = 0;
}
