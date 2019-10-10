#include "Gameengine.hpp"
#include "Gamestate.hpp"


void CGameEngine::Init(std::string const &_name)
{
	m_Window.Init(_name);

	m_CursorManager.Init(&m_Window);

	nextGameState = NULL;

	nextAction = action::hold;
	m_running = true;
	m_simpleRenderLoop = false;
}



void CGameEngine::Quit()
{
	ClearStates();

	if(nextGameState)
		nextGameState->Cleanup();

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
			lag += static_cast<double>(g_pTimer->GetElapsedTime().asMicroseconds()) / 1000.0;
			//updates
			while (lag >= MS_PER_UPDATE)
			{
				m_pStates.back()->HandleEvents();
				m_pStates.back()->Update();
				m_CursorManager.Update();
				//checks if the current state needs to be popped
				CheckStates();
				lag -= MS_PER_UPDATE;
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
	_state->Init(this);
	m_pStates.push_back(_state);
}



void CGameEngine::PopState()
{
	nextAction = action::pop;
}


void CGameEngine::ChangeState(GameState * _state)
{
	nextGameState = _state;
	nextAction = action::change;
}

void CGameEngine::ChangeStateImmediately(GameState * _state)
{
	ClearStates();
	_state->Init(this);
	m_pStates.push_back(_state);
}




void CGameEngine::ClearStates()
{
	//delete every state on the stack
	for (auto state : m_pStates)
	{
		state->Cleanup();
		SAFE_DELETE(state);
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
		m_pStates.back()->Cleanup();
		SAFE_DELETE(m_pStates.back());
		m_pStates.pop_back();
		break;

	case action::change:
		ChangeStateImmediately(nextGameState);
		break;
	}
	
	nextGameState = nullptr;
	nextAction = action::hold;
	
}
