#include "VendingMachinePanel.hpp"
#include "Resources\SoundManager.hpp"

void VendingMachinePanel::Init(GameStatus * _gameStatus, CGameEngine * _engine)
{
	gameStatus = _gameStatus;
	engine = _engine;

	vendingMachinePanel.Load(g_pTextures->vendingMachinePanel);
	vendingMachinePanel.SetPos(150, 70);

	scrollableItemPanel.Init(engine, [&](InventoryItemWrapper* _item) {return OnItemFromItemPanelReceived(_item); });
	scrollableItemPanel.SetPos(vendingMachinePanel.GetRect().left + 75, vendingMachinePanel.GetRect().top + 60);

	itemRowPanel.Init(engine, [&](InventoryItemWrapper* _item) {return OnItemFromConsumablePanelReceived(_item); });
	itemRowPanel.SetPos(vendingMachinePanel.GetRect().left + 100, vendingMachinePanel.GetRect().top + 700);

	shopPanel.Init(engine, gameStatus->GetConsumablesAvailability());
	shopPanel.SetPos(vendingMachinePanel.GetRect().left + 840, vendingMachinePanel.GetRect().top + 145);

	buttonClose.Load(g_pTextures->lootablePanelCloseButton, Buttontypes::Motion_Up);
	buttonClose.SetPos(vendingMachinePanel.GetGlobalRect().left + 1472, vendingMachinePanel.GetGlobalRect().top + 26);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonBuy.Load(g_pTextures->bangGenericButton, Buttontypes::Motion_Up);
	buttonBuy.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });
	buttonBuy.SetButtonstring("Kaufen");
	buttonBuy.SetButtontextFont(g_pFonts->f_trajan);
	buttonBuy.SetButtontextCharactersize(25);
	buttonBuy.SetPos(vendingMachinePanel.GetRect().left + 1050, vendingMachinePanel.GetRect().top + 740);

	title.setCharacterSize(50);
	title.setFont(g_pFonts->f_kingArthur);
	title.setFillColor(sf::Color::Black);
	title.setString("Shop");
	title.setPosition(vendingMachinePanel.GetRect().left + 1050, vendingMachinePanel.GetRect().top + 40);

	closed = true;
}

void VendingMachinePanel::Quit()
{
	scrollableItemPanel.Quit();
	itemRowPanel.Quit();
	shopPanel.Quit();
}

void VendingMachinePanel::Update()
{
	if (!closed)
	{
		if (buttonClose.Update(*engine) == true)
			closed = true;

		if (buttonBuy.Update(*engine) == true)
		{
			auto item = shopPanel.RetrieveCurrentlySelectedItem();

			if (item.id != ItemID::empty)
			{
				gameStatus->AddItem(item);

				if (item.id < CONSUMABLE_ITEMS_START)
					scrollableItemPanel.AddItem(item);
				else
					itemRowPanel.AddItem(item);
			}
		}

		if (shopPanel.IsItemSelected())
			buttonBuy.SetEnabled();
		else
			buttonBuy.SetDisabled();

		scrollableItemPanel.Update();
		itemRowPanel.Update();
		shopPanel.Update();
	}
}

void VendingMachinePanel::Render()
{
	if (!closed)
	{
		vendingMachinePanel.Render(engine->GetWindow());
		buttonClose.Render(engine->GetWindow());
		buttonBuy.Render(engine->GetWindow());

		engine->GetWindow().draw(title);

		shopPanel.Render();

		scrollableItemPanel.Render();
		itemRowPanel.Render();
		scrollableItemPanel.RenderCurrentlyDraggedItem();
		itemRowPanel.RenderCurrentlyDraggedItem();
	}
}


void VendingMachinePanel::Open()
{
	closed = false;

	scrollableItemPanel.Quit();
	for (auto &i : gameStatus->GetItems())
		scrollableItemPanel.AddItem(i);

	itemRowPanel.Quit();
	for (auto &c : gameStatus->GetConsumables())
		itemRowPanel.AddItem(c);

	shopPanel.ChooseNewRandomItems(1);
}

InventoryItemWrapper* VendingMachinePanel::OnItemFromItemPanelReceived(InventoryItemWrapper* _receivedItem)
{
	return _receivedItem;
}

InventoryItemWrapper * VendingMachinePanel::OnItemFromConsumablePanelReceived(InventoryItemWrapper * _receivedItem)
{
	return _receivedItem;
}
