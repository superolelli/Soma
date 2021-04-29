#include "StringInput.hpp"



void CStringInput::Init(sf::Font const &_font, int _characterSize, sf::Color _colour)
{
	//set the text's attributes
	m_text.setCharacterSize(_characterSize);
	m_text.setFillColor(_colour);
	m_text.setFont(_font);

	//set the backspace Timer
	m_backspaceTimer = 0.5;
	m_cursorTimer = 0.5;

	m_showCursor = true;

	//clear the string
	m_str.clear();

	//set up the cursor
	m_line[0].color = _colour;
	m_line[1].color = _colour;

	m_maximumCharacters = -1;
}

void CStringInput::SetPos(int _x, int _y)
{
	m_text.setPosition(static_cast<float>(_x), static_cast<float>(_y));

	m_line[0].position = sf::Vector2f(static_cast<float>(_x), static_cast<float>(_y));
	m_line[1].position = sf::Vector2f(static_cast<float>(_x), static_cast<float>(_y + m_text.getCharacterSize()));

	m_xPos = _x;
	m_yPos = _y;
}

void CStringInput::SetMaximumCharacters(int _maximum)
{
	m_maximumCharacters = _maximum;
}

void CStringInput::HandleInput(CGameEngine &_engine)
{
	//retrieve the text input
	auto input = _engine.GetTextInput();
	
	//check for backspace
	if (_engine.GetKeystates(KeyID::BackSpace) == Pressed && m_str.size() > 0)
		m_str.erase(m_str.end() - 1);
	else if (_engine.GetKeystates(KeyID::BackSpace) == Held && m_str.size() > 0)
	{
		//if backspace is held: Delete a letter every 0.5 seconds
		m_backspaceTimer -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
		if (m_backspaceTimer <= 0.0)
		{
			m_backspaceTimer = 0.05;
			m_str.erase(m_str.end() - 1);
		}
	}
	else if (_engine.GetKeystates(KeyID::BackSpace) == Released)
		m_backspaceTimer = 0.5;

	
	//if text was entered: add it to the string
	if (input != "" && (m_str.size() < m_maximumCharacters || m_maximumCharacters < 0))
	{
		//catch all invalid inputs
		if (!(_engine.GetKeystates(KeyID::BackSpace) == Pressed || _engine.GetKeystates(KeyID::BackSpace) == Held))
			m_str += input;
	}

	CheckCursor();
}



void CStringInput::Render(sf::RenderTarget &_target)
{
	//set the data
	m_text.setString(m_str);
	m_text.setPosition(static_cast<float>(m_xPos - m_text.getGlobalBounds().width/2), static_cast<float>(m_yPos));

	m_line[0].position = sf::Vector2f(static_cast<float>(m_text.getPosition().x + m_text.getGlobalBounds().width + 5), static_cast<float>(m_yPos + 5));
	m_line[1].position = sf::Vector2f(m_line[0].position.x, static_cast<float>(m_yPos + 5 + m_text.getCharacterSize()));

	//draw the string
	_target.draw(m_text);

	//draw the cursor
	if(m_showCursor)
		_target.draw(m_line, 2, sf::Lines);
}



void CStringInput::CheckCursor()
{
	//check the cursor timer
	m_cursorTimer -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
	if (m_cursorTimer <= 0.0)
	{
		//reset the timer
		m_cursorTimer = 0.5;

		//invert the value
		m_showCursor = !m_showCursor;
	}
}