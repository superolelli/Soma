#pragma once

#include "../Framework/Gameengine.hpp"
#include "../Framework/Gui/Button.hpp"
#include "../Framework/Graphics/SpriteWithText.hpp"

class ConfirmationDialog
{
public:

	ConfirmationDialog(CGameEngine* _engine, const sf::String &_text, int _fontSize, int _id = 0);

	void Update();
	void Render();

	void SetOpen(bool _open) { isOpen = _open; }
	void SetPos(int _x, int _y);

	bool IsOpen() { return isOpen; }
	bool YesChosen() { return yesChosen; }
	int ConfirmationID() { return confirmationID; }

private:
	CGameEngine* engine;

	int confirmationID;

	bool isOpen;
	bool yesChosen;

	SpriteWithText panel;
	CButton buttonYes;
	CButton buttonNo;
};