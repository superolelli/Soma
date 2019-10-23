#include "EquipmentPanel.hpp"



void EquipmentPanel::Init(CGameEngine * _engine, GameStatus *_gameStatus, int _xPos, int _yPos)
{
	engine = _engine;
	gameStatus = _gameStatus;

	for (auto &e : equipmentField)
		e.Load(g_pTextures->inventoryEquipmentField);

	equipmentField[0].SetPos(_xPos, _yPos);
	equipmentField[1].SetPos(_xPos + 365, _yPos);
	equipmentField[2].SetPos(_xPos, _yPos + 300);
	equipmentField[3].SetPos(_xPos + 365, _yPos + 300);

	tooltip.Init(engine);

	currentPlayer = 0;
	currentDraggedItem = -1;
}


void EquipmentPanel::Quit()
{
	for (int p = 0; p < 4; p++)
	{
		for(int i = 0; i < 4; i++)
		{
			SAFE_DELETE(items[p][i]);
		}
	}
}


void EquipmentPanel::SetInventoryRect(sf::IntRect &_inventoryRect)
{
	inventoryRect = _inventoryRect;
}


void EquipmentPanel::Update()
{
	HandleDragAndDrop();
}


void EquipmentPanel::HandleDragAndDrop()
{
	HandleDragStarted();
	HandleContinuedDrag();
	HandleDrop();
}


void EquipmentPanel::HandleDragStarted()
{
	if (engine->GetButtonstates(ButtonID::Left) != Pressed)
		return;

	for (int i = 0; i < 4; i++)
	{
		if (items[currentPlayer][i] != nullptr && items[currentPlayer][i]->Contains(engine->GetMousePos()))
		{
			currentDraggedItemOldX = items[currentPlayer][i]->GetGlobalBounds().left;
			currentDraggedItemOldY = items[currentPlayer][i]->GetGlobalBounds().top;
			currentDraggedItem = i;
		}
	}
}


void EquipmentPanel::HandleContinuedDrag()
{
	if (engine->GetButtonstates(ButtonID::Left) == Held && currentDraggedItem != -1)
		items[currentPlayer][currentDraggedItem]->SetCenterPos(engine->GetMousePos().x, engine->GetMousePos().y);
}


void EquipmentPanel::HandleDrop()
{
	if (engine->GetButtonstates(ButtonID::Left) == Released && currentDraggedItem != -1)
	{
		if (items[currentPlayer][currentDraggedItem]->GetGlobalBounds().intersects(inventoryRect))
			PlaceCurrentDraggedItemIntoInventory();
		else
			items[currentPlayer][currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);

		currentDraggedItem = -1;
	}
}


void EquipmentPanel::PlaceCurrentDraggedItemIntoInventory()
{
	gameStatus->RemoveEquipment(currentPlayer, currentDraggedItem);
	gameStatus->AddItem(items[currentPlayer][currentDraggedItem]->GetItem());
	SAFE_DELETE(items[currentPlayer][currentDraggedItem]);
}


void EquipmentPanel::Render()
{
	for (auto &e : equipmentField)
		e.Render(engine->GetWindow());

	RenderItems();
}


void EquipmentPanel::RenderItems()
{
	for (int i = 0; i < 4; i++)
	{
		if (items[currentPlayer][i] != nullptr && i != currentDraggedItem)
		{
			items[currentPlayer][i]->Render(engine->GetWindow());
			if (items[currentPlayer][i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
			{
				tooltip.SetItemID(items[currentPlayer][i]->GetItem().id);
				tooltip.ShowTooltip(engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
			}
		}
	}

	if (currentDraggedItem != -1)
		items[currentPlayer][currentDraggedItem]->Render(engine->GetWindow());
}


InventoryItemWrapper * EquipmentPanel::PlaceItem(InventoryItemWrapper *_item)
{
	for (int i = 0; i < 4; i++)
	{
		if (_item->GetGlobalBounds().intersects(equipmentField[i].GetGlobalRect()))
		{
			auto oldItem = items[currentPlayer][i];
			items[currentPlayer][i] = _item;
			items[currentPlayer][i]->SetPos(equipmentField[i].GetGlobalRect().left + 10, equipmentField[i].GetGlobalRect().top + 10);

			gameStatus->AddEquipment(currentPlayer, i, _item->GetItem());

			return oldItem;
		}
	}

	return nullptr;
}


bool EquipmentPanel::CanBePlaced(CSprite & _itemSprite)
{
	for (auto &e : equipmentField)
	{
		if (_itemSprite.GetGlobalRect().intersects(e.GetGlobalRect()))
			return true;
	}

	return false;
}


void EquipmentPanel::SetCurrentPlayer(int _currentPlayer)
{
	currentPlayer = _currentPlayer;
}
