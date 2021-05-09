#pragma once


#include <vector>
#include <memory>
#include "Time.hpp"
#include "Window.hpp"
#include "CursorManager.hpp"

class GameState;

#define MS_PER_UPDATE 12
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

	void UseSimpleRenderLoop(bool _simple) { m_simpleRenderLoop = _simple; double lag = 0.0;}

	//Returns the render window
	sf::RenderWindow &GetWindow() { return *m_Window.GetRenderWindow();}

	sf::RenderTarget& GetRenderTarget() { return m_RenderTextures[currentRenderTexture]; }

	void FlushRenderTarget(sf::Shader *_shader = nullptr);

	void ApplyShaderToRenderTarget(sf::Shader* _shader);

	void ClearRenderTarget(sf::Color const& _color) { m_RenderTextures[currentRenderTexture].clear(_color); }

	//Returns the size of the render window
	sf::Vector2u const &GetWindowSize() { return m_Window.GetRenderWindow()->getSize(); }

	//Returns the position of the upper left corner of the view
	sf::Vector2f GetViewPosition() { return GetRenderTarget().getView().getCenter() - GetRenderTarget().getView().getSize() / 2.0f; }

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
	inline sf::Vector2i const GetWorldMousePos() { return (sf::Vector2i)m_RenderTextures[currentRenderTexture].mapPixelToCoords(m_Window.GetMousePos()); }
	inline sf::Vector2f const &GetMouseMovement() { return m_Window.GetMouseMovement(); }

	void SetCursor(sf::Cursor::Type _type) { m_CursorManager.SetCursor(_type); }


private:
	// the stack of states
	std::vector<GameState*> m_pStates;

	//the window
	CWindow m_Window;
	sf::RenderTexture m_RenderTextures[2];
	int currentRenderTexture;

	CursorManager m_CursorManager;

	bool m_running;
	bool m_simpleRenderLoop;

	GameState *nextGameState;
	enum class action {hold, push, pop, change} nextAction;

	//clears all states
	void ClearStates();

	//checks if a state needs to be popped
	void CheckStates();
};


