#pragma once


#include "SFML\Graphics.hpp"
#include "Makros.hpp"


enum Keystates{None, Pressed, Held, Released};

//typedef for the key ids
typedef sf::Keyboard::Key KeyID;
typedef sf::Mouse::Button ButtonID;

class CWindow
{
public:

	//Inits the window 
	void Init(std::string const &_title);

	//Quits everything safe
	void Quit();

	//Fills the background black
	void Clear(const sf::Color &_color) const;

	//flips the double-buffer and shows the new frame
	void Flip();

	//Processes Input
	void ProcessEvents();

	//returns a pointer to the window
	inline sf::RenderWindow *GetRenderWindow() { return m_pWindow; }

	//The getters
	Keystates const &GetKeystates(KeyID _id) const { return m_keyStates[_id]; }
	Keystates const &GetButtonstates(ButtonID _id) const { return m_mouseStates[_id]; }
	inline std::string &GetTextInput() { return m_textInput; }
	inline int const GetMouseWheelMovement() const { return m_mouseWheelMovement; }
	inline sf::Vector2i const &GetMousePos()  { return m_mousePosition; }


private:
	//arrays for saving the key- and buttonstates
	Keystates m_keyStates[sf::Keyboard::KeyCount];
	Keystates m_mouseStates[sf::Mouse::ButtonCount];
	std::string m_textInput;
	int m_mouseWheelMovement;
	sf::Vector2i m_mousePosition;

	//the window to render on
	sf::RenderWindow *m_pWindow;

	//takes the old keystates into account
	void ResetKeyStates();

	//completeley resets the keystates
	void ClearKeyStates();
};