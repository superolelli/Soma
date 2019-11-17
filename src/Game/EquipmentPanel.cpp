#include "EquipmentPanel.hpp"
#include "Resources\SoundManager.hpp"


void EquipmentPanel::Init(CGameEngine * _engine, GameStatus *_gameStatus, int _xPos, int _yPos)
{
	engine = _engine;
	gameStatus = _gameStatus;

	for (auto &e : equipmentField)
		e.Load(g_pTextures->inventoryEquipmentField);

	equipmentField[0].SetPos(_xPos, _yPos);
	equipmentField[1].SetPos(_xPos + 365, _yPos);
	equipmentField[2].SetPos(_xPos + 365, _yPos + 300);
	equipmentField[3].SetPos(_xPos, _yPos + 300);

	connectionsBackground.Load(g_pTextures->inventoryConnectionsBackground);
	connectionsBackground.SetPos(_xPos + 45, _yPos + 30);

	diamond.Load(g_pTextures->inventoryDiamond);
	diamond.SetPos(_xPos + 185, _yPos + 158);
	diamond.SetColor(0, 0, 0);

	diamondUpdateTime = 0.0f;

	connections[0].Init(0);
	connections[0].SetPos(_xPos + 104, _yPos + 36);

	connections[1].Init(1);
	connections[1].SetPos(_xPos + 285, _yPos + 78);

	connections[2].Init(2);
	connections[2].SetPos(_xPos + 104, _yPos + 252);

	connections[3].Init(3);
	connections[3].SetPos(_xPos + 50, _yPos + 79);

	tooltip.Init();

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

	for (auto &c : connections)
		c.Update();

	UpdateDiamond();
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
		{
			PlaceCurrentDraggedItemIntoInventory();
			
			connections[currentDraggedItem].DeactivateConnection(0);
			connections[LeftNeighbourSlot(currentDraggedItem)].DeactivateConnection(1);

			diamondUpdateTime = 4.0f;
			RecolorDiamond();
		}
		else
			items[currentPlayer][currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);

		currentDraggedItem = -1;
	}
}


void EquipmentPanel::PlaceCurrentDraggedItemIntoInventory()
{
	g_pSounds->PlaySound(soundID::INVENTORY_DROP);
	gameStatus->RemoveEquipment(currentPlayer, currentDraggedItem);
	gameStatus->AddItem(items[currentPlayer][currentDraggedItem]->GetItem());
	SAFE_DELETE(items[currentPlayer][currentDraggedItem]);
}


void EquipmentPanel::Render()
{
	connectionsBackground.Render(engine->GetWindow());

	for(auto &c : connections)
		c.Render(engine->GetWindow());

	diamond.Render(engine->GetWindow());

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
				tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
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

			CheckConnections(i);
			diamondUpdateTime = 4.0f;
			RecolorDiamond();

			return oldItem;
		}
	}

	return nullptr;
}


void EquipmentPanel::CheckConnections(int _slot)
{
	auto itemColor = items[currentPlayer][_slot]->GetItem().color;

	if (items[currentPlayer][LeftNeighbourSlot(_slot)] && itemColor == items[currentPlayer][LeftNeighbourSlot(_slot)]->GetItem().color)
		connections[LeftNeighbourSlot(_slot)].ActivateConnection(itemColor);
	else
		connections[LeftNeighbourSlot(_slot)].DeactivateConnection(1);

	if (items[currentPlayer][RightNeighbourSlot(_slot)] && itemColor == items[currentPlayer][RightNeighbourSlot(_slot)]->GetItem().color)
		connections[_slot].ActivateConnection(itemColor);
	else
		connections[_slot].DeactivateConnection(0);
}


void EquipmentPanel::UpdateDiamond()
{
	if (diamondUpdateTime > 0.0f)
	{
		float t = (4.0f - diamondUpdateTime)/4.0f;
		diamondUpdateTime -= g_pTimer->GetElapsedTime().asSeconds();

		//lerp
		int r = (1.0f - t) * oldDiamondColor.r + t * newDiamondColor.r;
		int g = (1.0f - t) * oldDiamondColor.g + t * newDiamondColor.g;
		int b = (1.0f - t) * oldDiamondColor.b + t * newDiamondColor.b;

		diamond.SetColor(r, g, b);
	}
}

void EquipmentPanel::RecolorDiamond()
{
	int r = 0;
	int g = 0;
	int b = 0;

	for (auto &c : connections)
	{
		r = std::min(255, r + c.GetColor().r / 2);
		g = std::min(255, g + c.GetColor().g / 2);
		b = std::min(255, b + c.GetColor().b / 2);
	}

	oldDiamondColor = diamond.GetColor();
	newDiamondColor = sf::Color(r, g, b);
}

int EquipmentPanel::RightNeighbourSlot(int _slot)
{
	_slot += 1;

	if (_slot > 3)
		_slot = 0;

	return _slot;
}

int EquipmentPanel::LeftNeighbourSlot(int _slot)
{
	_slot -= 1;

	if (_slot < 0)
		_slot = 3;

	return _slot;
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
