#pragma once

#include "Window.hpp"


class CursorManager {
public:

	void Init(CWindow *_window);
	void Update();
	void SetCursor(sf::Cursor::Type _type);

private:
	CWindow *window;

	sf::Image cursorNormalImage;
	sf::Image cursorHandImage;

	sf::Cursor cursorNormal;
	sf::Cursor cursorHand;

	sf::Cursor::Type currentCursorType;
};