#include "Inventory.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

void Inventory::Init(GameStatus * _gameStatus, CGameEngine * _engine)
{
	gameStatus = _gameStatus;
	engine = _engine;

	currentPlayer = 0;

	inventoryPanel.Load(g_pTextures->inventoryPanel);
	inventoryPanel.SetPos(143, 69);

	ScrollableItemPanel *newScrollableItemPanel = new ScrollableItemPanel;
	newScrollableItemPanel->Init(engine);
	scrollableItemPanel.Init(engine, newScrollableItemPanel);
	scrollableItemPanel.SetOnItemDroppedCallback([&](InventoryItemWrapper* _item) {return OnItemFromItemPanelReceived(_item); });
	scrollableItemPanel.SetPos(inventoryPanel.GetRect().left + 843, inventoryPanel.GetRect().top + 212);

	buttonSortNames.Load(g_pTextures->sortNamesButton, Buttontypes::Up);
	buttonSortNames.SetPos(scrollableItemPanel.GetRect().left + scrollableItemPanel.GetRect().width + 11, scrollableItemPanel.GetRect().top - 14);
	buttonSortNames.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonSortColors.Load(g_pTextures->sortColorButton, Buttontypes::Up);
	buttonSortColors.SetPos(scrollableItemPanel.GetRect().left + scrollableItemPanel.GetRect().width + 11, buttonSortNames.GetRect().top + buttonSortNames.GetRect().height - 2);
	buttonSortColors.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonNext.Load(g_pTextures->skillPanelButtonNext, Buttontypes::Up);
	buttonNext.SetPos(inventoryPanel.GetGlobalRect().left + 217, inventoryPanel.GetGlobalRect().top + 66);
	buttonNext.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonPrevious.Load(g_pTextures->skillPanelButtonPrevious, Buttontypes::Up);
	buttonPrevious.SetPos(inventoryPanel.GetGlobalRect().left + 15, inventoryPanel.GetGlobalRect().top + 66);
	buttonPrevious.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonClose.Load(g_pTextures->skillPanelButtonClose, Buttontypes::Motion_Up);
	buttonClose.SetPos(inventoryPanel.GetGlobalRect().left + 1475, inventoryPanel.GetGlobalRect().top + 66);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	currentPlayerName.setCharacterSize(47);
	currentPlayerName.setFont(g_pFonts->f_blackwoodCastle);
	currentPlayerName.setFillColor(sf::Color::Black);
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(inventoryPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, inventoryPanel.GetGlobalRect().top + 55);

	panelTitle.setCharacterSize(70);
	panelTitle.setFont(g_pFonts->f_blackwoodCastle);
	panelTitle.setFillColor(sf::Color::Black);
	panelTitle.setString("Inventar");
	panelTitle.setPosition(inventoryPanel.GetGlobalRect().left + 850, inventoryPanel.GetGlobalRect().top + 70);

	equipmentPanel.Init(engine, gameStatus, inventoryPanel.GetRect().left + 309, inventoryPanel.GetRect().top + 287);
	equipmentPanel.SetOnItemDroppedCallback([&](InventoryItemWrapper* _item, int _currentPlayer, int _equipmentId) {return OnItemFromEquipmentPanelReceived(_item, _currentPlayer, _equipmentId); });

	playerAttributesText.SetPos(inventoryPanel.GetRect().left + 85, inventoryPanel.GetRect().top + 408);

	closed = true;
}


void Inventory::Quit()
{
	scrollableItemPanel.Quit();
	equipmentPanel.Quit();
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

		playerAttributesText.Update(gameStatus->GetEquipmentStats(currentPlayer) + g_pObjectProperties->playerStats[currentPlayer]);
	}
}


InventoryItemWrapper * Inventory::OnItemFromItemPanelReceived(InventoryItemWrapper * _receivedItem)
{
	if (equipmentPanel.CanBePlaced(_receivedItem->GetSprite()))
	{
		g_pSounds->PlaySound(soundID::INVENTORY_DROP);
		gameStatus->RemoveItem(_receivedItem->GetItem());
		auto returnItem = equipmentPanel.PlaceItem(_receivedItem);

		if (returnItem != nullptr)
		{
			gameStatus->AddItem(returnItem->GetItem());
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
		gameStatus->RemoveEquipment(_currentPlayer, _equipmentId);
		gameStatus->AddItem(_receivedItem->GetItem());
		scrollableItemPanel.AddItem(_receivedItem->GetItem());
		SAFE_DELETE(_receivedItem);
	}
	return _receivedItem;
}



void Inventory::UpdateGUIForChosenPlayer()
{
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
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
	for (auto &i : gameStatus->GetItems())
		scrollableItemPanel.AddItem(i);
}




