#pragma once

#include "GUI.hpp"
#include "SkillPanel.hpp"
#include "Inventory.hpp"
#include "VendingMachinePanel.hpp"
#include "ResourcesStatusBar.hpp"

class MainRoomGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine, GameStatus *_gameStatus);
	virtual void Update() override;
	virtual void Render() override;
	void Quit();

	void PlayerClicked(int _id);
	void VendingMachineClicked();

	bool IsPanelOpen();

private:
	CGameEngine *engine;
	GameStatus *gameStatus;

	SkillPanel skillPanel;
	Inventory inventory;
	VendingMachinePanel vendingMachinePanel;
	ResourcesStatusBar resourcesStatusBar;

	CButton inventoryButton;
	CButton skillpanelButton;
	CButton shopButton;

	void CloseAllPanels();

};