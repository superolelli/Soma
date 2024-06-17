#include "Inventory.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

Inventory::Inventory(CGameEngine * _engine)
	: engine(_engine)
	, currentPlayer(0)
	, inventoryPanel(g_pTextures->inventoryPanel)
	, scrollableItemPanel(_engine, new ScrollableItemPanel(_engine))
	, buttonSortNames(g_pTextures->sortNamesButton, Buttontypes::Up)
	, buttonSortColors(g_pTextures->sortColorButton, Buttontypes::Up)
	, buttonNext(g_pTextures->skillPanelButtonNext, Buttontypes::Up)
	, buttonPrevious(g_pTextures->skillPanelButtonPrevious, Buttontypes::Up)
	, buttonClose(g_pTextures->skillPanelButtonClose, Buttontypes::Motion_Up)
	, equipmentPanel(_engine)
{
	inventoryPanel.SetPos(143, 69);

	scrollableItemPanel.SetOnItemDroppedCallback([&](InventoryItemWrapper* _item) {return OnItemFromItemPanelReceived(_item); });
	scrollableItemPanel.SetPos(inventoryPanel.GetRect().left + 843, inventoryPanel.GetRect().top + 212);

	buttonSortNames.SetPos(scrollableItemPanel.GetRect().left + scrollableItemPanel.GetRect().width + 11, scrollableItemPanel.GetRect().top - 14);
	buttonSortNames.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonSortColors.SetPos(scrollableItemPanel.GetRect().left + scrollableItemPanel.GetRect().width + 11, buttonSortNames.GetRect().top + buttonSortNames.GetRect().height - 2);
	buttonSortColors.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonNext.SetPos(inventoryPanel.GetGlobalRect().left + 217, inventoryPanel.GetGlobalRect().top + 66);
	buttonNext.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonPrevious.SetPos(inventoryPanel.GetGlobalRect().left + 15, inventoryPanel.GetGlobalRect().top + 66);
	buttonPrevious.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonClose.SetPos(inventoryPanel.GetGlobalRect().left + 1475, inventoryPanel.GetGlobalRect().top + 66);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	currentPlayerName.setCharacterSize(47);
	currentPlayerName.setFont(g_pFonts->f_blackwoodCastle);
	currentPlayerName.setFillColor(sf::Color::Black);
	currentPlayerName.setString(g_pObjectProperties->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(inventoryPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, inventoryPanel.GetGlobalRect().top + 55);

	panelTitle.setCharacterSize(70);
	panelTitle.setFont(g_pFonts->f_blackwoodCastle);
	panelTitle.setFillColor(sf::Color::Black);
	panelTitle.setString("Inventar");
	panelTitle.setPosition(inventoryPanel.GetGlobalRect().left + 850, inventoryPanel.GetGlobalRect().top + 70);

	equipmentPanel.SetPos(inventoryPanel.GetRect().left + 309, inventoryPanel.GetRect().top + 287);
	equipmentPanel.SetOnItemDroppedCallback([&](InventoryItemWrapper* _item, int _currentPlayer, int _equipmentId) {return OnItemFromEquipmentPanelReceived(_item, _currentPlayer, _equipmentId); });

	playerAttributesText.SetPos(inventoryPanel.GetRect().left + 65, inventoryPanel.GetRect().top + 330);

	closed = true;
}

void Inventory::Update()
{
	if (!closed)
	{
		if (buttonClose.Update(*engine) == true)
			closed = true;

		if (buttonSortColors.Update(*engine) == true)
			scrollableItemPanel.SortItemsAccordingToColor();

		if (buttonSortNames.Update(*engine) == true)
			scrollableItemPanel.SortItemsAccordingToNames();

		CheckButtonsForPlayerChoosing();
		equipmentPanel.Update();

		scrollableItemPanel.Update();

		playerAttributesText.Update(g_pGameStatus->GetEquipmentStats(currentPlayer).stats + g_pObjectProperties->playerStats[currentPlayer] + g_pGameStatus->GetDiamondStats(currentPlayer));
	}
}


InventoryItemWrapper * Inventory::OnItemFromItemPanelReceived(InventoryItemWrapper * _receivedItem)
{
	if (equipmentPanel.CanBePlaced(_receivedItem->GetSprite()))
	{
		g_pSounds->PlaySound(soundID::INVENTORY_DROP);
		g_pGameStatus->RemoveItem(_receivedItem->GetItem());
		auto returnItem = equipmentPanel.PlaceItem(_receivedItem);

		if (returnItem != nullptr)
		{
			g_pGameStatus->AddItem(returnItem->GetItem());
			return returnItem;
		}

		return nullptr;
	}

	return _receivedItem;
}

InventoryItemWrapper * Inventory::OnItemFromEquipmentPanelReceived(InventoryItemWrapper * _receivedItem, int _currentPlayer, int _equipmentId)
{
	if (_receivedItem->GetGlobalBounds().intersects(scrollableItemPanel.GetRect()))
	{
		g_pSounds->PlaySound(soundID::INVENTORY_DROP);
		g_pGameStatus->RemoveEquipment(_currentPlayer, _equipmentId);
		g_pGameStatus->AddItem(_receivedItem->GetItem());
		scrollableItemPanel.AddItem(_receivedItem->GetItem());
		SAFE_DELETE(_receivedItem);
	}
	return _receivedItem;
}



void Inventory::UpdateGUIForChosenPlayer()
{
	currentPlayerName.setString(g_pObjectProperties->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(inventoryPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, inventoryPanel.GetGlobalRect().top + 55);
	equipmentPanel.SetCurrentPlayer(currentPlayer);
}


void Inventory::OnItemAdded(Item _item)
{
	scrollableItemPanel.AddItem(_item);
}


void Inventory::CheckButtonsForPlayerChoosing()
{
	int lastCurrentPlayer = currentPlayer;

	if (buttonNext.Update(*engine) == true)
		currentPlayer++;

	if (buttonPrevious.Update(*engine) == true)
		currentPlayer--;

	if (currentPlayer > 3)
		currentPlayer = 0;
	if (currentPlayer < 0)
		currentPlayer = 3;

	if (currentPlayer != lastCurrentPlayer)
	{
		UpdateGUIForChosenPlayer();
		equipmentPanel.SetCurrentPlayer(currentPlayer);
	}
}

void Inventory::Render()
{
	if (!closed)
	{
		inventoryPanel.Render(engine->GetRenderTarget());

		buttonSortColors.Render(engine->GetRenderTarget());
		buttonSortNames.Render(engine->GetRenderTarget());

		scrollableItemPanel.Render();
		equipmentPanel.Render();
		scrollableItemPanel.RenderCurrentlyDraggedItem();
		equipmentPanel.RenderCurrentDraggedItem();

		playerAttributesText.Render(engine->GetRenderTarget());

		engine->GetRenderTarget().draw(currentPlayerName);
		engine->GetRenderTarget().draw(panelTitle);

		buttonNext.Render(engine->GetRenderTarget());
		buttonPrevious.Render(engine->GetRenderTarget());
		buttonClose.Render(engine->GetRenderTarget());
	}
}


void Inventory::Open(int _player)
{
	closed = false;
	currentPlayer = _player;
	UpdateGUIForChosenPlayer();

	scrollableItemPanel.Clear();
	for (auto &i : g_pGameStatus->GetItems())
		scrollableItemPanel.AddItem(i);

	equipmentPanel.Clear();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			equipmentPanel.AddItem(g_pGameStatus->GetEquipment(i, j), i, j);
		}
	}
}




