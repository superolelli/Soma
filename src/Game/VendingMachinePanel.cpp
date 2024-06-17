#include "VendingMachinePanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources/ObjectPropertiesManager.hpp"

VendingMachinePanel::VendingMachinePanel(CGameEngine * _engine)
	: engine(_engine)
	, dialog(nullptr)
	, shopPanel(_engine)
	, closed(true)
	, vendingMachinePanel(g_pTextures->vendingMachinePanel)
	, cardsSymbol(g_pTextures->cardsSymbol)
	, scrollableItemPanel(engine, new ScrollableItemPanel(engine))
	, itemRowPanel(engine, new ItemRowPanel(engine))
	, buttonSortNames(g_pTextures->sortNamesButton, Buttontypes::Up)
	, buttonSortColors(g_pTextures->sortColorButton, Buttontypes::Up)
	, buttonClose(g_pTextures->lootablePanelCloseButton, Buttontypes::Motion_Up)
	, buttonBuy(g_pTextures->bangGenericButton, Buttontypes::Motion_Up)
{
	vendingMachinePanel.SetPos(150, 70);

	scrollableItemPanel.SetPos(vendingMachinePanel.GetRect().left + 75, vendingMachinePanel.GetRect().top + 60);
	scrollableItemPanel.SetOnItemSelectedCallback([&](Item &_item) {return OnItemOfScrollablePanelSelected(_item); });

	itemRowPanel.SetPos(vendingMachinePanel.GetRect().left + 100, vendingMachinePanel.GetRect().top + 700);
	itemRowPanel.SetOnItemSelectedCallback([&](Item &_item) {return OnItemOfRowPanelSelected(_item); });

	shopPanel.SetOnItemSelectedCallback([&](Item &_item) {return OnItemOfShopPanelSelected(_item); });
	shopPanel.SetPos(vendingMachinePanel.GetRect().left + 840, vendingMachinePanel.GetRect().top + 130);

	buttonSortNames.SetPos(scrollableItemPanel.GetRect().left + scrollableItemPanel.GetRect().width + 25, scrollableItemPanel.GetRect().top - 21);
	buttonSortNames.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonSortColors.SetPos(scrollableItemPanel.GetRect().left + scrollableItemPanel.GetRect().width + 25, buttonSortNames.GetRect().top + buttonSortNames.GetRect().height - 2);
	buttonSortColors.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonClose.SetPos(vendingMachinePanel.GetGlobalRect().left + 1472, vendingMachinePanel.GetGlobalRect().top + 26);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonBuy.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });
	buttonBuy.SetButtonstring("Kaufen");
	buttonBuy.SetButtontextFont(g_pFonts->f_trajan);
	buttonBuy.SetButtontextCharactersize(25);
	buttonBuy.SetPos(vendingMachinePanel.GetRect().left + 1050, vendingMachinePanel.GetRect().top + 725);
	buttonBuy.SetDisabled();

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

	shopPanel.ChooseNewRandomItems(g_pGameStatus->levels[LevelType::bang], g_pGameStatus->levels[LevelType::kutschfahrt], g_pGameStatus->levels[LevelType::tichu]);
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

		if (buttonSortColors.Update(*engine) == true)
			scrollableItemPanel.SortItemsAccordingToColor();

		if (buttonSortNames.Update(*engine) == true)
			scrollableItemPanel.SortItemsAccordingToNames();

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
	g_pGameStatus->RemoveCards(shopPanel.CurrentItemPrice());
	auto item = shopPanel.RetrieveCurrentlySelectedItem();

	if (item.id != ItemID::empty)
	{
		g_pGameStatus->AddItem(item);

		if (item.id < CONSUMABLE_ITEMS_START)
			scrollableItemPanel.AddItem(item);
		else
		{
			itemRowPanel.AddItem(item);
			if (shopPanel.CurrentItemPrice() > g_pGameStatus->GetCardsAmount())
				buttonBuy.SetDisabled();
		}
	}
}


void VendingMachinePanel::OpenSellMultipleDialog()
{
	dialog = new SellMultipleDialog(engine);
	dialog->SetItem(itemRowPanel.CurrentlySelectedItem());
}

void VendingMachinePanel::Render()
{
	if (!closed)
	{
		vendingMachinePanel.Render(engine->GetRenderTarget());
		buttonClose.Render(engine->GetRenderTarget());
		buttonBuy.Render(engine->GetRenderTarget());

		engine->GetRenderTarget().draw(title);

		shopPanel.Render();

		buttonSortColors.Render(engine->GetRenderTarget());
		buttonSortNames.Render(engine->GetRenderTarget());

		scrollableItemPanel.Render();
		itemRowPanel.Render();

		if (shopPanel.IsItemSelected() || scrollableItemPanel.IsItemSelected() || itemRowPanel.IsItemSelected())
		{
			engine->GetRenderTarget().draw(priceText);
			cardsSymbol.Render(engine->GetRenderTarget());
		}

		if(dialog != nullptr)
			dialog->Render();
	}
}


void VendingMachinePanel::ChooseNewShopItems()
{
	shopPanel.ChooseNewRandomItems(g_pGameStatus->levels[LevelType::bang], g_pGameStatus->levels[LevelType::kutschfahrt], g_pGameStatus->levels[LevelType::tichu]);
}

void VendingMachinePanel::Open()
{
	closed = false;

	scrollableItemPanel.Clear();
	for (auto &i : g_pGameStatus->GetItems())
		scrollableItemPanel.AddItem(i);

	itemRowPanel.Clear();
	for (auto &c : g_pGameStatus->GetConsumables())
		itemRowPanel.AddItem(c);
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

	if ((_item.id < CONSUMABLE_ITEMS_START || g_pGameStatus->GetConsumablesAvailability().at(_item.id)) && g_pObjectProperties->getItemStats(_item.id).price <= g_pGameStatus->GetCardsAmount())
		buttonBuy.SetEnabled();
	else
		buttonBuy.SetDisabled();

	itemRowPanel.UnselectSelectedItem();
	scrollableItemPanel.UnselectSelectedItem();
	priceText.setString("Für " + std::to_string(g_pObjectProperties->getItemStats(_item.id).price));
	priceText.setPosition(buttonBuy.GetRect().left + buttonBuy.GetRect().width / 2 - priceText.getGlobalBounds().width / 2 - 20, priceText.getPosition().y);
	cardsSymbol.SetPos(priceText.getGlobalBounds().left + priceText.getGlobalBounds().width + 10, priceText.getGlobalBounds().top - 9);
}


void VendingMachinePanel::SellItem(Item const &_item, int _amount)
{
	int buyingPrice = g_pObjectProperties->getItemStats(_item.id).price;
	g_pGameStatus->AddCards(buyingPrice * _amount / 5);
	g_pGameStatus->RemoveItem(_item, _amount <= 1);
}
