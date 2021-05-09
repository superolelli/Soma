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

	for(auto &d : diamond)
		d.Init(engine, _xPos + 185, _yPos + 158);

	for (auto &c : connections)
	{
		c[0].Init(0);
		c[0].SetPos(_xPos + 104, _yPos + 36);
		c[1].Init(1);
		c[1].SetPos(_xPos + 285, _yPos + 78);
		c[2].Init(2);
		c[2].SetPos(_xPos + 104, _yPos + 252);
		c[3].Init(3);
		c[3].SetPos(_xPos + 50, _yPos + 79);
	}

	tooltip.Init();

	currentPlayer = 0;
	currentDraggedItem = -1;
}


void EquipmentPanel::Quit()
{
	Clear();
}

void EquipmentPanel::SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*, int, int)> _onItemDropped)
{
	OnItemDropped = _onItemDropped;
}


void EquipmentPanel::Update()
{
	HandleDragAndDrop();

	for (auto &c : connections[currentPlayer])
		c.Update();

	diamond[currentPlayer].Update();
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
		items[currentPlayer][currentDraggedItem] = OnItemDropped(items[currentPlayer][currentDraggedItem], currentPlayer, currentDraggedItem);
		if (items[currentPlayer][currentDraggedItem] == nullptr)
		{
			connections[currentPlayer][currentDraggedItem].DeactivateConnection(0);
			connections[currentPlayer][LeftNeighbourSlot(currentDraggedItem)].DeactivateConnection(1);

			diamond[currentPlayer].RecolorDiamond(connections[currentPlayer]);
			gameStatus->SetDiamondStats(currentPlayer, diamond[currentPlayer].GetStats());
		}
		else
			items[currentPlayer][currentDraggedItem]->SetPos(currentDraggedItemOldX, currentDraggedItemOldY);

		currentDraggedItem = -1;
	}
}


void EquipmentPanel::Render()
{
	connectionsBackground.Render(engine->GetRenderTarget());

	for(auto &c : connections[currentPlayer])
		c.Render(engine->GetRenderTarget());

	diamond[currentPlayer].Render();

	for (auto &e : equipmentField)
		e.Render(engine->GetRenderTarget());

	RenderItems();
}


void EquipmentPanel::RenderItems()
{
	for (int i = 0; i < 4; i++)
	{
		if (items[currentPlayer][i] != nullptr && i != currentDraggedItem)
		{
			items[currentPlayer][i]->Render(engine->GetRenderTarget());
			if (items[currentPlayer][i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
			{
				tooltip.SetItem(items[currentPlayer][i]->GetItem().id);
				tooltip.ShowTooltip(engine->GetRenderTarget(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
			}
		}
	}
}


void EquipmentPanel::RenderCurrentDraggedItem()
{
	if (currentDraggedItem != -1)
		items[currentPlayer][currentDraggedItem]->Render(engine->GetRenderTarget());
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
			diamond[currentPlayer].RecolorDiamond(connections[currentPlayer]);
			gameStatus->SetDiamondStats(currentPlayer, diamond[currentPlayer].GetStats());

			return oldItem;
		}
	}

	return nullptr;
}


void EquipmentPanel::CheckConnections(int _slot)
{
	auto itemColor = items[currentPlayer][_slot]->GetItem().color;

	if (items[currentPlayer][LeftNeighbourSlot(_slot)] && itemColor == items[currentPlayer][LeftNeighbourSlot(_slot)]->GetItem().color)
		connections[currentPlayer][LeftNeighbourSlot(_slot)].ActivateConnection(itemColor);
	else
		connections[currentPlayer][LeftNeighbourSlot(_slot)].DeactivateConnection(1);

	if (items[currentPlayer][RightNeighbourSlot(_slot)] && itemColor == items[currentPlayer][RightNeighbourSlot(_slot)]->GetItem().color)
		connections[currentPlayer][_slot].ActivateConnection(itemColor);
	else
		connections[currentPlayer][_slot].DeactivateConnection(0);
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

void EquipmentPanel::Clear()
{
	for (int i = 0; i < 4; i++) 
	{
		for (int j = 0; j < 4; j++)
		{
			SAFE_DELETE(items[i][j]);
		}
	}
}

void EquipmentPanel::AddItem(Item& _item, int player, int slot)
{
	if (_item.id == ItemID::empty)
		return;

	SAFE_DELETE(items[player][slot]);

	auto newItem = new InventoryItemWrapper;

	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);
	newItem->Init(std::move(_item), std::move(newSprite));
	newItem->SetPos(equipmentField[slot].GetGlobalRect().left + 10, equipmentField[slot].GetGlobalRect().top + 10);

	items[player][slot] = newItem;

	auto tempCurrentPlayer = currentPlayer;
	SetCurrentPlayer(player);
	CheckConnections(slot);
	diamond[player].RecolorDiamond(connections[player]);
	gameStatus->SetDiamondStats(player, diamond[player].GetStats());
	SetCurrentPlayer(tempCurrentPlayer);
}


void EquipmentPanel::SetCurrentPlayer(int _currentPlayer)
{
	currentPlayer = _currentPlayer;
}
