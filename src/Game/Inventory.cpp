#include "Inventory.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

void Inventory::Init(GameStatus * _gameStatus, CGameEngine * _engine)
{
	gameStatus = _gameStatus;
	gameStatus->SetOnItemAddedCallback([&](Item _item) {OnItemAdded(_item); });
	engine = _engine;

	currentPlayer = 0;
	currentUpperRow = 0;

	currentDraggedItem = -1;

	inventoryPanel.Load(g_pTextures->inventoryPanel);
	inventoryPanel.SetPos(150, 70);

	scrollbar.Init(35, 563);
	scrollbar.SetColor(sf::Color(28, 28, 28));
	scrollbar.SetPos(inventoryPanel.GetGlobalRect().left + 1423, inventoryPanel.GetGlobalRect().top + 200);

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

	tooltip.Init();
	equipmentPanel.Init(engine, gameStatus, 300, 330);
	equipmentPanel.SetInventoryRect(sf::IntRect(inventoryPanel.GetGlobalRect().left + 812, inventoryPanel.GetGlobalRect().top + 171, 700, 610));

	closed = true;
}


void Inventory::Quit()
{
	for (auto &i : items)
		SAFE_DELETE(i);

	equipmentPanel.Quit();
}


void Inventory::Update()
{
	if (!closed)
	{
		if (buttonClose.Update(*engine) == true)
			closed = true;

		UpdateScrollbar();
		CheckButtonsForPlayerChoosing();
		HandleDragAndDrop();
		equipmentPanel.Update();
	}
}


void Inventory::UpdateScrollbar()
{
	if (currentDraggedItem == -1)
		scrollbar.Update(*engine);

	if (scrollbar.GetCurrentStep() != currentUpperRow)
	{
		currentUpperRow = scrollbar.GetCurrentStep();
		RecalculatePositionsOfItems();
	}
}


void Inventory::HandleDragAndDrop()
{
	HandleStartedDrag();
	HandleContinuedDrag();
	HandleDrop();
}


void Inventory::HandleStartedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) != Pressed)
		return;

	int x = engine->GetMousePos().x - (inventoryPanel.GetGlobalRect().left + 832);
	int y = engine->GetMousePos().y - (inventoryPanel.GetGlobalRect().top + 195);
	int itemPointedTo = (y / 116 + currentUpperRow) * 5 + x / 116;

	if (itemPointedTo >= 0 && items.size() > itemPointedTo && items[itemPointedTo] != nullptr)
	{
		if (items[itemPointedTo]->Contains(engine->GetMousePos()))
		{
			currentDraggedItemOldX = items[itemPointedTo]->GetGlobalBounds().left;
			currentDraggedItemOldY = items[itemPointedTo]->GetGlobalBounds().top;
			currentDraggedItem = itemPointedTo;
		}
	}
}


void Inventory::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		items[currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}



void Inventory::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		if (equipmentPanel.CanBePlaced(items[currentDraggedItem]->GetSprite()))
			PlaceCurrentDraggedItemAsEquipment();
		else
			items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);

		currentDraggedItem = -1;
	}
}



void Inventory::PlaceCurrentDraggedItemAsEquipment()
{
	g_pSounds->PlaySound(soundID::INVENTORY_DROP);
	gameStatus->RemoveItem(items[currentDraggedItem]->GetItem());
	items[currentDraggedItem] = equipmentPanel.PlaceItem(items[currentDraggedItem]);

	if (items[currentDraggedItem] != nullptr)
	{
		items[currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);
		gameStatus->AddItem(items[currentDraggedItem]->GetItem(), false);
	}
}


void Inventory::RecalculatePositionsOfItems()
{
	int yPos = inventoryPanel.GetGlobalRect().top + 195;
	int currentIndex = 0;

	for (int y = 0; y < 5; y++)
	{
		int xPos = inventoryPanel.GetGlobalRect().left + 832;
		for (int x = 0; x < 5; x++)
		{
			currentIndex = (currentUpperRow + y) * 5 + x;
			if(currentIndex < items.size() && items[currentIndex] != nullptr)
				items[currentIndex]->SetPos(xPos, yPos);

			xPos += 116;
		}
		yPos += 116;
	}
}


void Inventory::UpdateGUIForChosenPlayer()
{
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(inventoryPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, inventoryPanel.GetGlobalRect().top + 55);
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
		equipmentPanel.Render();
		scrollbar.Render(engine->GetWindow());

		engine->GetWindow().draw(currentPlayerName);
		engine->GetWindow().draw(panelTitle);

		buttonNext.Render(engine->GetWindow());
		buttonPrevious.Render(engine->GetWindow());
		buttonClose.Render(engine->GetWindow());

		RenderItems();
	}
}



void Inventory::RenderItems()
{
	int showTooltipForItem = -1;
	for (int i = currentUpperRow * 5; i < currentUpperRow * 5 + 25 && i < items.size(); i++)
	{
		if (items[i] != nullptr && i != currentDraggedItem)
		{
			items[i]->Render(engine->GetWindow());

			if (items[i]->Contains(engine->GetMousePos()))
				showTooltipForItem = items[i]->GetItem().id;
		}
	}

	if (currentDraggedItem != -1)
		items[currentDraggedItem]->Render(engine->GetWindow());

	if (showTooltipForItem != -1)
	{
		tooltip.SetItem(ItemID(showTooltipForItem));
		tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
	}
}



void Inventory::Open(int _player)
{
	closed = false;
	currentPlayer = _player;
	UpdateGUIForChosenPlayer();
}


void Inventory::OnItemAdded(Item _item)
{
	InventoryItemWrapper *newItem = new InventoryItemWrapper;

	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);
	newItem->Init(std::move(_item), std::move(newSprite));

	int freeSlot = GetFirstFreeInventorySlot();
	int xPos = (freeSlot % 5) * 116 + inventoryPanel.GetGlobalRect().left + 832;
	int yPos = (freeSlot / 5) * 116 + inventoryPanel.GetGlobalRect().top + 195;
	newItem->SetPos(xPos, yPos);

	if (freeSlot < items.size())
		items[freeSlot] = newItem;
	else
		items.push_back(newItem);

	scrollbar.SetNumberOfSteps(std::max(1, (int)(items.size() - 21) / 5 + 1));
}



int Inventory::GetFirstFreeInventorySlot()
{
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] == nullptr)
			return i;
	}

	return items.size();
}