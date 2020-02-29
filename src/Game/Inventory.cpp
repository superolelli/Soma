#include "Inventory.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

void Inventory::Init(GameStatus * _gameStatus, CGameEngine * _engine)
{
	gameStatus = _gameStatus;
	gameStatus->SetOnItemAddedCallback([&](Item _item) {OnItemAdded(_item); });
	engine = _engine;

	currentPlayer = 0;

	inventoryPanel.Load(g_pTextures->inventoryPanel);
	inventoryPanel.SetPos(150, 70);

	scrollableItemPanel.Init(engine, [&](InventoryItemWrapper* _item) {return OnItemFromItemPanelReceived(_item); });
	scrollableItemPanel.SetPos(inventoryPanel.GetRect().left + 822, inventoryPanel.GetRect().top + 185);

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

	equipmentPanel.Init(engine, gameStatus, 300, 330);
	equipmentPanel.SetInventoryRect(sf::IntRect(inventoryPanel.GetGlobalRect().left + 812, inventoryPanel.GetGlobalRect().top + 171, 700, 610));

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

		CheckButtonsForPlayerChoosing();
		equipmentPanel.Update();

		scrollableItemPanel.Update();
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
			gameStatus->AddItem(returnItem->GetItem(), false);
			return returnItem;
		}

		return nullptr;
	}

	return _receivedItem;
}



void Inventory::UpdateGUIForChosenPlayer()
{
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(inventoryPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, inventoryPanel.GetGlobalRect().top + 55);
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
		inventoryPanel.Render(engine->GetWindow());

		scrollableItemPanel.Render();
		equipmentPanel.Render();
		scrollableItemPanel.RenderCurrentlyDraggedItem();
		equipmentPanel.RenderCurrentDraggedItem();

		engine->GetWindow().draw(currentPlayerName);
		engine->GetWindow().draw(panelTitle);

		buttonNext.Render(engine->GetWindow());
		buttonPrevious.Render(engine->GetWindow());
		buttonClose.Render(engine->GetWindow());
	}
}


void Inventory::Open(int _player)
{
	closed = false;
	currentPlayer = _player;
	UpdateGUIForChosenPlayer();

	scrollableItemPanel.Quit();
	for (auto &i : gameStatus->GetItems())
		scrollableItemPanel.AddItem(i);
}




