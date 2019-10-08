#include "Button.hpp"



CButton::CButton()
{
}


CButton::~CButton()
{
}


void CButton::SetDisabled()
{
	isEnabled = false;
	m_ButtonSprite.SetColor(80, 80, 80);
}

void CButton::SetEnabled()
{
	isEnabled = true;
	m_ButtonSprite.SetColor(255, 255, 255);
}


void CButton::SetCallback(std::function<void()> _callback)
{
	m_Callback = _callback;
}

//Loads the texture, sets the position and type of the button
void CButton::Load(sf::Texture const &_texture, Buttontypes _type, std::string const &_str)
{
	//(int)(type/2) is 2 for up/down and 3 for motion_up/motion_down buttons
	m_ButtonSprite.Load(_texture, 2 + (int)(static_cast<int>(_type) / 2), _texture.getSize().x / (2 + (int)(static_cast<int>(_type) / 2)), _texture.getSize().y);

	//set the button's position and type
	m_ButtonSprite.SetPos(0, 0);
	m_Buttontype = _type;

	nextFrame = 0.0f;

	isEnabled = true;

	//sets the buttonstring
	m_Buttonstring = _str;
	m_Buttontext.setString(m_Buttonstring);

	SetDefaultButtontext();

	m_Buttontext.setPosition(m_ButtonSprite.GetRect().left + (m_ButtonSprite.GetRect().width - m_Buttontext.getGlobalBounds().width) / 2, m_ButtonSprite.GetRect().top + (m_ButtonSprite.GetRect().height - m_Buttontext.getGlobalBounds().height) / 2);
}


void CButton::SetDefaultButtontext()
{
	//TODO: set default font
	m_Buttontext.setFillColor(sf::Color::Black);
	m_Buttontext.setStyle(sf::Text::Style::Regular);
	m_Buttontext.setCharacterSize(30);
}


//sets the button's position
void CButton::SetPos(int _x, int _y)
{
	m_ButtonSprite.SetPos(_x, _y);
	m_Buttontext.setPosition(m_ButtonSprite.GetRect().left + (m_ButtonSprite.GetRect().width - m_Buttontext.getGlobalBounds().width) / 2, m_ButtonSprite.GetRect().top + (m_ButtonSprite.GetRect().height - m_Buttontext.getGlobalBounds().height) / 2);
}


void CButton::SetScale(float _x, float _y)
{
	m_ButtonSprite.SetScale(_x, _y);
	m_Buttontext.setCharacterSize(static_cast<int>(static_cast<float>(m_Buttontext.getCharacterSize())* _x));
}


void CButton::SetButtontextFont(sf::Font const &_font)
{
	m_Buttontext.setFont(_font);
}


void CButton::SetButtontextCharactersize(int _size)
{
	m_Buttontext.setCharacterSize(_size);
}


void CButton::SetButtontextColor(sf::Color const &_color)
{
	m_Buttontext.setFillColor(_color);
}


void CButton::SetButtontextStyle(sf::Text::Style _style)
{
	m_Buttontext.setStyle(_style);
}

//sets the buttonstring
void CButton::SetButtonstring(std::string const &_buttonstring)
{
	m_Buttonstring = _buttonstring;
	m_Buttontext.setString(_buttonstring);

	m_Buttontext.setPosition(m_ButtonSprite.GetRect().left + (m_ButtonSprite.GetRect().width - m_Buttontext.getGlobalBounds().width) / 2, m_ButtonSprite.GetRect().top + (m_ButtonSprite.GetRect().height - m_Buttontext.getGlobalBounds().height) / 2);
}



bool CButton::Update(CGameEngine &_engine)
{
	nextFrame = 0.0f;
	bool buttonWasPressed = ButtonWasPressed(_engine);

	if (buttonWasPressed && m_Callback)
		m_Callback();

	return buttonWasPressed;
}


bool CButton::ButtonWasPressed(CGameEngine &_engine)
{
	//if the mouse is at the button
	if (isEnabled && m_ButtonSprite.GetRect().contains(_engine.GetMousePos()))
	{
		_engine.SetCursor(sf::Cursor::Hand);

		//If the left mouse button was released
		if (_engine.GetButtonstates(ButtonID::Left) == Released)
		{
			//render the right button frame
			switch (m_Buttontype)
			{
			case Buttontypes::Up:
				nextFrame = 1.0f;
				return true;
				break;

			case Buttontypes::Down:
				nextFrame = 0.0f;
				return false;
				break;

			case Buttontypes::Motion_Up:
				nextFrame = 2.0f;
				return true;
				break;

			case Buttontypes::Motion_Down:
				nextFrame = 1.0;
				return false;
				break;
			}
		}
		//if the left mouse button was pressed or is held
		else if (_engine.GetButtonstates(ButtonID::Left) == Pressed || _engine.GetButtonstates(ButtonID::Left) == Held)
		{
			//Render the right button frame
			switch (m_Buttontype)
			{
			case Buttontypes::Up:
				nextFrame = 1.0f;
				return false;
				break;

			case Buttontypes::Down:
				nextFrame = 1.0f;
				return true;
				break;

			case Buttontypes::Motion_Up:
				nextFrame = 2.0f;
				return false;
				break;

			case Buttontypes::Motion_Down:
				nextFrame = 2.0f;
				return true;
				break;
			}
		}
		else
		{
			if (m_Buttontype == Buttontypes::Up || m_Buttontype == Buttontypes::Down)
			{
				nextFrame = 0.0f;
			}
			else
				nextFrame = 1.0f;

			return false;
		}
	}

	return false;
}

void CButton::Render(CGameEngine &_engine)
{
	m_ButtonSprite.Render(_engine.GetWindow(), nextFrame);
	RenderButtontext(_engine);
}




void CButton::RenderButtontext(CGameEngine & _engine)
{
	if (m_Buttonstring != "")
		_engine.GetWindow().draw(m_Buttontext);
}



