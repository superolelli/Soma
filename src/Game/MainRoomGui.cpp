#include "MainRoomGui.hpp"
#include "Resources\SoundManager.hpp"

void MainRoomGUI::Init(CGameEngine * _engine, GameStatus *_gameStatus)
{
	engine = _engine;
	gameStatus = _gameStatus;
	resourcesStatusBar.Init(engine);

	skillPanel.Init(gameStatus, engine);
	inventory.Init(gameStatus, engine);
	vendingMachinePanel.Init(gameStatus, engine);

	inventoryButton.Load(g_pTextures->mainRoomButtonInventory, Buttontypes::Up);
	inventoryButton.SetPos(0, 0);
	inventoryButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	skillpanelButton.Load(g_pTextures->mainRoomButtonSkillPanel, Buttontypes::Up);
	skillpanelButton.SetPos(53, 0);
	skillpanelButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	shopButton.Load(g_pTextures->mainRoomButtonShop, Buttontypes::Up);
	shopButton.SetPos(106, 0);
	shopButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

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

	if (inventoryButton.Update(*engine) == true)
	{
		CloseAllPanels();
		inventory.Open();
	}

	if (skillpanelButton.Update(*engine) == true)
	{
		CloseAllPanels();
		skillPanel.Open();
	}

	if (shopButton.Update(*engine) == true)
	{
		CloseAllPanels();
		vendingMachinePanel.Open();
	}
}

void MainRoomGUI::Render()
{
	resourcesStatusBar.Render();
	skillPanel.Render();
	inventory.Render();
	vendingMachinePanel.Render();

	inventoryButton.Render(engine->GetRenderTarget());
	skillpanelButton.Render(engine->GetRenderTarget());
	shopButton.Render(engine->GetRenderTarget());
}


void MainRoomGUI::CloseAllPanels()
{
	inventory.Close();
	skillPanel.Close();
	vendingMachinePanel.Close();
}
