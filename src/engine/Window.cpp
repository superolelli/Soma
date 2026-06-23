#include "Window.hpp"


void CWindow::Init(std::string const &_title)
{
	using namespace sf;

	//make a new window
	m_pWindow = new RenderWindow;
	m_pWindow->create(VideoMode::getDesktopMode(), _title, Style::None, State::Fullscreen, sf::ContextSettings{0U, 0U, 0U, 3U, 3U});

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
	//clears all the keystates
	ResetKeyStates();

	//sets all keystates
	while (const auto event = m_pWindow->pollEvent())
	{
		if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
			m_textInput = sf::String(textEntered->unicode).toAnsiString();
		else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			const auto key = static_cast<std::size_t>(keyPressed->code);
			if (m_keyStates[key] == Keystates::Pressed || m_keyStates[key] == Keystates::Held)
				m_keyStates[key] = Keystates::Held;
			else
				m_keyStates[key] = Keystates::Pressed;
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
			m_keyStates[static_cast<std::size_t>(keyReleased->code)] = Keystates::Released;
		else if (const auto* buttonPressed = event->getIf<sf::Event::MouseButtonPressed>())
		{
			const auto button = static_cast<std::size_t>(buttonPressed->button);
			if (m_mouseStates[button] == Keystates::Pressed || m_mouseStates[button] == Keystates::Held)
				m_mouseStates[button] = Keystates::Held;
			else
				m_mouseStates[button] = Keystates::Pressed;
		}
		else if (const auto* buttonReleased = event->getIf<sf::Event::MouseButtonReleased>())
			m_mouseStates[static_cast<std::size_t>(buttonReleased->button)] = Keystates::Released;
		else if (const auto* wheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())
			m_mouseWheelMovement = static_cast<int>(wheelScrolled->delta);
		else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
			m_mouseMovement = sf::Vector2f(mouseMoved->position - m_mousePosition);
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
