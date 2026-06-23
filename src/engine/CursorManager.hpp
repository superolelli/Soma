#pragma once

#include "Window.hpp"
#include <optional>


class CursorManager {
public:

	void Init(CWindow *_window);
	void Update();
	void SetCursor(sf::Cursor::Type _type);

private:
	CWindow *window;

	sf::Image cursorNormalImage;
	sf::Image cursorHandImage;

	std::optional<sf::Cursor> cursorNormal;
	std::optional<sf::Cursor> cursorHand;

	sf::Cursor::Type currentCursorType;
};
