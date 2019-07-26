#include "CursorManager.hpp"



void CursorManager::Init(CWindow * _window)
{
	window = _window;
	cursorNormalImage.loadFromFile("Data/Cursors/brown/left_ptr.png");
	cursorHandImage.loadFromFile("Data/Cursors/brown/pointing_hand.png");

	cursorNormal.loadFromPixels(cursorNormalImage.getPixelsPtr(), cursorNormalImage.getSize(), {0, 0});
	cursorHand.loadFromPixels(cursorHandImage.getPixelsPtr(), cursorHandImage.getSize(), { 0, 0 });

	window->SetMouseCursor(cursorNormal);
}



void CursorManager::Update()
{
	switch (currentCursorType) {
	case sf::Cursor::Type::Arrow:
		window->SetMouseCursor(cursorNormal);
		break;
	case sf::Cursor::Type::Hand:
		window->SetMouseCursor(cursorHand);
		break;
	default:
		window->SetMouseCursor(cursorNormal);
		break;
	}

	currentCursorType = sf::Cursor::Type::Arrow;
}



void CursorManager::SetCursor(sf::Cursor::Type _type)
{
	currentCursorType = _type;
}
