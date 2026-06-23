#include "MainRoomGui.hpp"
#include "Resources\SoundManager.hpp"

MainRoomGUI::MainRoomGUI(CGameEngine* _engine)
	: engine(_engine)
	, resourcesStatusBar(_engine)
	, skillPanel(_engine)
	, inventory(_engine)
	, vendingMachinePanel(_engine)
	, inventoryButton(g_pTextures->mainRoomButtonInventory, Buttontypes::Up)
	, skillpanelButton(g_pTextures->mainRoomButtonSkillPanel, Buttontypes::Up)
	, shopButton(g_pTextures->mainRoomButtonShop, Buttontypes::Up)
{
	inventoryButton.SetPos(0, 0);
	inventoryButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	skillpanelButton.SetPos(53, 0);
	skillpanelButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	shopButton.SetPos(106, 0);
	shopButton.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });
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
	resourcesStatusBar.Update(g_pGameStatus->GetCardsAmount(), g_pGameStatus->GetDiceAmount());

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
