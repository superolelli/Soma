#include "MainRoomGui.hpp"


void MainRoomGUI::Init(CGameEngine * _engine, GameStatus *_gameStatus)
{
	engine = _engine;
	gameStatus = _gameStatus;
	resourcesStatusBar.Init(engine);

	skillPanel.Init(gameStatus, engine);
	inventory.Init(gameStatus, engine);
	vendingMachinePanel.Init(gameStatus, engine);

}

void MainRoomGUI::Quit()
{
	resourcesStatusBar.Quit();
	inventory.Quit();
	vendingMachinePanel.Quit();
}

void MainRoomGUI::PlayerClicked(int _id)
{
	skillPanel.Open(_id);
}

void MainRoomGUI::VendingMachineClicked()
{
	vendingMachinePanel.Open();
}

bool MainRoomGUI::IsPanelOpen()
{
	return inventory.IsOpen() || vendingMachinePanel.IsOpen() || skillPanel.IsOpen();
}

void MainRoomGUI::Update()
{
	skillPanel.Update();
	inventory.Update();
	vendingMachinePanel.Update();
	resourcesStatusBar.Update(gameStatus->GetCardsAmount(), gameStatus->GetDiceAmount());

	if (engine->GetKeystates(KeyID::I) == Keystates::Released && IsPanelOpen())
		inventory.Open();
}

void MainRoomGUI::Render()
{
	resourcesStatusBar.Render();
	skillPanel.Render();
	inventory.Render();
	vendingMachinePanel.Render();
}
