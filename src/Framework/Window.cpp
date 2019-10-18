#include "Window.hpp"


void CWindow::Init(std::string const &_title)
{
	using namespace sf;

	//make a new window
	m_pWindow = new RenderWindow;
	m_pWindow->create(VideoMode::getDesktopMode(), _title, Style::None | Style::Fullscreen);

	ClearKeyStates();

	m_pWindow->setVerticalSyncEnabled(true);
	m_pWindow->setKeyRepeatEnabled(false);
}



void CWindow::Quit()
{
	m_pWindow->close();
	SAFE_DELETE(m_pWindow);
}



void CWindow::Clear(const sf::Color &_color) const
{
	m_pWindow->clear(_color);
} 



void CWindow::Flip()
{
	m_pWindow->display();
}




void CWindow::ProcessEvents()
{
	sf::Event event;

	//clears all the keystates
	ResetKeyStates();

	//sets all keystates
	while (m_pWindow->pollEvent(event))
	{
		//check for text input
		if (event.type == sf::Event::TextEntered)
			m_textInput = event.text.unicode;
		

		//all other events
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			if (m_keyStates[event.key.code] == Keystates::Pressed || m_keyStates[event.key.code] == Keystates::Held)
				m_keyStates[event.key.code] = Keystates::Held;
			else
				m_keyStates[event.key.code] = Keystates::Pressed;
			break;

		case sf::Event::KeyReleased:
			m_keyStates[event.key.code] = Keystates::Released;
			break;

		case sf::Event::MouseButtonPressed:
			if (m_mouseStates[event.key.code] == Keystates::Pressed || m_mouseStates[event.key.code] == Keystates::Held)
				m_mouseStates[event.key.code] = Keystates::Held;
			else
				m_mouseStates[event.key.code] = Keystates::Pressed;
			break;

		case sf::Event::MouseButtonReleased:
			m_mouseStates[event.key.code] = Keystates::Released;
			break;

		case sf::Event::MouseWheelMoved:
			m_mouseWheelMovement = event.mouseWheel.delta;
			break;

		case sf::Event::MouseMoved:
			m_mouseMovement = sf::Vector2f(event.mouseMove.x - m_mousePosition.x, event.mouseMove.y - m_mousePosition.y);
		}
	}

	//gets the current mouse position
	m_mousePosition = sf::Mouse::getPosition(*m_pWindow);
}




void CWindow::ResetKeyStates()
{
	for (Keystates &k : m_keyStates)
	{
		if (k == Keystates::Pressed || k == Keystates::Held)
			k = Keystates::Held;
		else
			k = Keystates::None;
	}

	for (Keystates &k : m_mouseStates)
	{
		if (k == Keystates::Pressed || k == Keystates::Held)
			k = Keystates::Held;
		else
			k = Keystates::None;
	}

	m_textInput.clear();
	m_mouseWheelMovement = 0;
	m_mouseMovement = sf::Vector2f(0, 0);
}





void CWindow::ClearKeyStates()
{
	for (Keystates &k : m_keyStates)
		k = Keystates::None;

	for (Keystates &k : m_mouseStates)
		k = Keystates::None;

	m_textInput.clear();
	m_mouseWheelMovement = 0;
	m_mouseMovement = sf::Vector2f(0, 0);
}
