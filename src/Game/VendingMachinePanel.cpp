#include "VendingMachinePanel.hpp"
#include "Resources\SoundManager.hpp"

void VendingMachinePanel::Init(GameStatus * _gameStatus, CGameEngine * _engine)
{
	gameStatus = _gameStatus;
	engine = _engine;

	dialog = nullptr;

	vendingMachinePanel.Load(g_pTextures->vendingMachinePanel);
	vendingMachinePanel.SetPos(150, 70);

	cardsSymbol.Load(g_pTextures->cardsSymbol);

	scrollableItemPanel.Init(engine);
	scrollableItemPanel.SetOnItemSelectedCallback([&](Item &_item) {return OnItemOfScrollablePanelSelected(_item); });
	scrollableItemPanel.SetPos(vendingMachinePanel.GetRect().left + 75, vendingMachinePanel.GetRect().top + 60);

	itemRowPanel.Init(engine);
	itemRowPanel.SetOnItemSelectedCallback([&](Item &_item) {return OnItemOfRowPanelSelected(_item); });
	itemRowPanel.SetPos(vendingMachinePanel.GetRect().left + 100, vendingMachinePanel.GetRect().top + 700);

	shopPanel.Init(engine, gameStatus->GetConsumablesAvailability());
	shopPanel.SetOnItemSelectedCallback([&](Item &_item) {return OnItemOfShopPanelSelected(_item); });
	shopPanel.SetPos(vendingMachinePanel.GetRect().left + 840, vendingMachinePanel.GetRect().top + 130);

	buttonClose.Load(g_pTextures->lootablePanelCloseButton, Buttontypes::Motion_Up);
	buttonClose.SetPos(vendingMachinePanel.GetGlobalRect().left + 1472, vendingMachinePanel.GetGlobalRect().top + 26);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonBuy.Load(g_pTextures->bangGenericButton, Buttontypes::Motion_Up);
	buttonBuy.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });
	buttonBuy.SetButtonstring("Kaufen");
	buttonBuy.SetButtontextFont(g_pFonts->f_trajan);
	buttonBuy.SetButtontextCharactersize(25);
	buttonBuy.SetPos(vendingMachinePanel.GetRect().left + 1050, vendingMachinePanel.GetRect().top + 725);

	title.setCharacterSize(50);
	title.setFont(g_pFonts->f_kingArthur);
	title.setFillColor(sf::Color::Black);
	title.setString("Shop");
	title.setPosition(vendingMachinePanel.GetRect().left + 1050, vendingMachinePanel.GetRect().top + 40);

	priceText.setCharacterSize(25);
	priceText.setFont(g_pFonts->f_blackwoodCastle);
	priceText.setFillColor(sf::Color::White);
	priceText.setString("Für 0");
	priceText.setPosition(buttonBuy.GetRect().left, buttonBuy.GetRect().top + 75);

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
		if (dialog != nullptr)
		{
			HandleDialog();
			return;
		}

		if (buttonClose.Update(*engine) == true)
			closed = true;

		if (buttonBuy.Update(*engine) == true)
		{
			if (shopPanel.IsItemSelected())
				BuyCurrentItemFromShop();
			else if (itemRowPanel.IsItemSelected())
			{
				if (itemRowPanel.CurrentlySelectedItem().number > 1)
					OpenSellMultipleDialog();
				else
					SellItem(itemRowPanel.RetrieveCurrentlySelectedItem());
			}
			else if (scrollableItemPanel.IsItemSelected())
			{
				SellItem(scrollableItemPanel.RetrieveCurrentlySelectedItem());
				buttonBuy.SetDisabled();
			}
			
		}

		scrollableItemPanel.Update();
		itemRowPanel.Update();
		shopPanel.Update();
	}
}


void VendingMachinePanel::HandleDialog()
{
	dialog->Update();
	if (!dialog->IsOpen())
	{
		auto item = itemRowPanel.CurrentlySelectedItem();
		item.number = dialog->GetAmount();
		SellItem(item, item.number);
		itemRowPanel.ReduceAmountOfCurrentlySelectedItem(item.number);

		SAFE_DELETE(dialog);
	}
}


void VendingMachinePanel::BuyCurrentItemFromShop()
{
	gameStatus->RemoveCards(shopPanel.CurrentItemPrice());
	auto item = shopPanel.RetrieveCurrentlySelectedItem();

	if (item.id != ItemID::empty)
	{
		gameStatus->AddItem(item);

		if (item.id < CONSUMABLE_ITEMS_START)
			scrollableItemPanel.AddItem(item);
		else
		{
			itemRowPanel.AddItem(item);
			if (shopPanel.CurrentItemPrice() > gameStatus->GetCardsAmount())
				buttonBuy.SetDisabled();
		}
	}
}


void VendingMachinePanel::OpenSellMultipleDialog()
{
	dialog = new SellMultipleDialog;
	dialog->Init(engine);
	dialog->SetItem(itemRowPanel.CurrentlySelectedItem());
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

		if (shopPanel.IsItemSelected() || scrollableItemPanel.IsItemSelected() || itemRowPanel.IsItemSelected())
		{
			engine->GetWindow().draw(priceText);
			cardsSymbol.Render(engine->GetWindow());
		}

		if(dialog != nullptr)
			dialog->Render();
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

void VendingMachinePanel::OnItemOfRowPanelSelected(Item &_item)
{
	buttonBuy.SetButtonstring("Verkaufen");
	buttonBuy.SetEnabled();
	shopPanel.UnselectSelectedItem();
	scrollableItemPanel.UnselectSelectedItem();
	priceText.setString("Für " + std::to_string(g_pObjectProperties->getItemStats(_item.id).price / 5));
	priceText.setPosition(buttonBuy.GetRect().left + buttonBuy.GetRect().width / 2 - priceText.getGlobalBounds().width / 2 - 20, priceText.getPosition().y);
	cardsSymbol.SetPos(priceText.getGlobalBounds().left + priceText.getGlobalBounds().width + 10, priceText.getGlobalBounds().top - 9);
}

void VendingMachinePanel::OnItemOfScrollablePanelSelected(Item &_item)
{
	buttonBuy.SetButtonstring("Verkaufen");
	buttonBuy.SetEnabled();
	shopPanel.UnselectSelectedItem();
	itemRowPanel.UnselectSelectedItem();
	priceText.setString("Für " + std::to_string(g_pObjectProperties->getItemStats(_item.id).price / 5));
	priceText.setPosition(buttonBuy.GetRect().left + buttonBuy.GetRect().width / 2 - priceText.getGlobalBounds().width / 2 - 20, priceText.getPosition().y);
	cardsSymbol.SetPos(priceText.getGlobalBounds().left + priceText.getGlobalBounds().width + 10, priceText.getGlobalBounds().top - 9);
}

void VendingMachinePanel::OnItemOfShopPanelSelected(Item &_item)
{
	buttonBuy.SetButtonstring("Kaufen");

	if ((_item.id < CONSUMABLE_ITEMS_START || gameStatus->GetConsumablesAvailability().at(_item.id)) && g_pObjectProperties->getItemStats(_item.id).price <= gameStatus->GetCardsAmount())
		buttonBuy.SetEnabled();
	else
		buttonBuy.SetDisabled();

	itemRowPanel.UnselectSelectedItem();
	scrollableItemPanel.UnselectSelectedItem();
	priceText.setString("Für " + std::to_string(g_pObjectProperties->getItemStats(_item.id).price));
	priceText.setPosition(buttonBuy.GetRect().left + buttonBuy.GetRect().width / 2 - priceText.getGlobalBounds().width / 2 - 20, priceText.getPosition().y);
	cardsSymbol.SetPos(priceText.getGlobalBounds().left + priceText.getGlobalBounds().width + 10, priceText.getGlobalBounds().top - 9);
}


void VendingMachinePanel::SellItem(Item &_item, int _amount)
{
	int buyingPrice = g_pObjectProperties->getItemStats(_item.id).price;
	gameStatus->AddCards(buyingPrice * _amount / 5);
	gameStatus->RemoveItem(_item, _amount <= 1);
}
