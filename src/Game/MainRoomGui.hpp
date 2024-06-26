#pragma once

#include "GUI.hpp"
#include "SkillPanel.hpp"
#include "Inventory.hpp"
#include "VendingMachinePanel.hpp"
#include "ResourcesStatusBar.hpp"

class MainRoomGUI : public GUI
{
public:

	MainRoomGUI(CGameEngine* _engine);

	virtual void Update() override;
	virtual void Render() override;

	void PlayerClicked(int _id);
	void VendingMachineClicked();

	bool IsPanelOpen();

	void ChooseNewShopItems() { vendingMachinePanel.ChooseNewShopItems(); }

private:
	CGameEngine *engine;

	SkillPanel skillPanel;
	Inventory inventory;
	VendingMachinePanel vendingMachinePanel;
	ResourcesStatusBar resourcesStatusBar;

	CButton inventoryButton;
	CButton skillpanelButton;
	CButton shopButton;

	void CloseAllPanels();

};