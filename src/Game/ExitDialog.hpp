#pragma once

#include "../Framework/Gui/Button.hpp"
#include "../Framework/Graphics/SpriteWithText.hpp"

class ExitDialog
{
public:

	void Init(CGameEngine* _engine);
	bool Update();
	void Render();
	void Quit();

	void SetPos(int _x, int _y);

	void Open() {isOpen = true;}
	bool IsOpen() { return isOpen; }

private:
	CGameEngine* engine;

	bool isOpen;

	SpriteWithText panel;
	CButton buttonYes;
	CButton buttonNo;
};