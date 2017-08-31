#pragma once

#include <SFML/Graphics.hpp>
#include "../Gameengine.hpp"

class CStringInput
{
public:

	//initializes the input field with all the data
	void Init(sf::Font const &_font, int _characterSize, int _xPos, int _yPos, sf::Color _colour = sf::Color::Black);

	//renders the input field
	void Render(sf::RenderTarget &_target);

	//handles the input
	void HandleInput(CGameEngine &_engine);

	//returns the current string
	std::string &GetString(){return m_str;}

	//sets the current string
	void SetString(std::string const &_str){m_str = _str;}


private:
	std::string m_str;            //the input string
	sf::Text m_text;
	sf::Vertex m_line[2];          //the cursor

	int m_xPos, m_yPos;

	float m_backspaceTimer;
	float m_cursorTimer;

	bool m_showCursor;

	void CheckCursor();
};

