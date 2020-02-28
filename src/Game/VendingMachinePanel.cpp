#include "VendingMachinePanel.hpp"
#include "Resources\SoundManager.hpp"

void VendingMachinePanel::Init(GameStatus * _gameStatus, CGameEngine * _engine)
{
	gameStatus = _gameStatus;
	engine = _engine;

	vendingMachinePanel.Load(g_pTextures->vendingMachinePanel);
	vendingMachinePanel.SetPos(150, 70);

	buttonClose.Load(g_pTextures->lootablePanelCloseButton, Buttontypes::Motion_Up);
	buttonClose.SetPos(vendingMachinePanel.GetGlobalRect().left + 1472, vendingMachinePanel.GetGlobalRect().top + 26);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	closed = true;
}

void VendingMachinePanel::Quit()
{
}

void VendingMachinePanel::Update()
{
	if (!closed)
	{
		if (buttonClose.Update(*engine) == true)
			closed = true;
	}
}

void VendingMachinePanel::Render()
{
	if (!closed)
	{
		vendingMachinePanel.Render(engine->GetWindow());
		buttonClose.Render(engine->GetWindow());
	}
}


void VendingMachinePanel::Open()
{
	closed = false;
}
