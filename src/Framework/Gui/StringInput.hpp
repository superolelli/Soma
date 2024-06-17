#pragma once

#include <SFML/Graphics.hpp>
#include "../Gameengine.hpp"

class CStringInput
{
public:

	CStringInput(sf::Font const& _font, int _characterSize = 20, sf::Color _colour = sf::Color::Black);

	//renders the input field
	void Render(sf::RenderTarget &_target);

	//handles the input
	void HandleInput(CGameEngine &_engine);

	//returns the current string
	std::string &GetString(){return m_str;}

	//sets the current string
	void SetString(std::string const& _str) { m_str = _str; m_text.setString(_str); }

	void SetPos(int _x, int _y);

	void SetMaximumCharacters(int _maximum);


private:
	std::string m_str;            //the input string
	sf::Text m_text;
	sf::Vertex m_line[2];          //the cursor

	int m_xPos, m_yPos;

	int m_maximumCharacters;

	double m_backspaceTimer;
	double m_cursorTimer;

	bool m_showCursor;

	void CheckCursor();
};

