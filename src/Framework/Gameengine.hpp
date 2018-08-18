#pragma once


#include <vector>
#include <memory>
#include "Time.hpp"
#include "Window.hpp"

class GameState;

#define MS_PER_UPDATE 16
#define SCREEN_SIZE_X 1920


class CGameEngine
{
public:

	void Init(std::string const &_name);
	void Run();
	void Quit();

	//changes the current state and deletes the whole stack
	void ChangeState(GameState* _state);

	void ChangeStateImmediately(GameState* _state);

	//pushes a new state to the stack
	void PushState(GameState* _state);

	void PushStateImmediately(GameState* _state);

	//pops the last state of the stack
	void PopState();

	//stops the engine
	void StopEngine() { m_running = false; }

	//Returns the render window
	sf::RenderWindow &GetWindow() { return *m_Window.GetRenderWindow();}

	//Returns the size of the render window
	sf::Vector2u &GetWindowSize() { return m_Window.GetRenderWindow()->getSize(); }

	//Returns the position of the upper left corner of the view
	sf::Vector2f GetViewPosition() { return GetWindow().getView().getCenter() - GetWindow().getView().getSize() / 2.0f; }

	//clears the window
	void ClearWindow(sf::Color const &_color) { m_Window.Clear(_color); }

	//Flips the window
	void FlipWindow() { m_Window.Flip(); }

	//processes the events
	void ProcessEvents() { m_Window.ProcessEvents(); }

	//getters for the key- and buttonstates
	Keystates const &GetKeystates(KeyID _id) const { return m_Window.GetKeystates(_id); }
	Keystates const &GetButtonstates(ButtonID _id) const { return m_Window.GetButtonstates(_id); }
	inline std::string &GetTextInput() { return m_Window.GetTextInput(); }
	inline int const GetMouseWheelMovement() const { return m_Window.GetMouseWheelMovement(); }
	inline sf::Vector2i const &GetMousePos() { return m_Window.GetMousePos(); }
	inline sf::Vector2i const GetWorldMousePos() { return m_Window.GetWorldMousePos(); }


private:
	// the stack of states
	std::vector<GameState*> m_pStates;

	//the window
	CWindow m_Window;

	bool m_running;

	GameState *nextGameState;
	enum class action {hold, push, pop, change} nextAction;

	//clears all states
	void ClearStates();

	//checks if a state needs to be popped
	void CheckStates();
};


