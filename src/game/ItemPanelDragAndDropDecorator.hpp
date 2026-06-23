#pragma once

#include "ItemPanelDecorator.hpp"

class ItemPanelDragAndDropDecorator : public ItemPanelDecorator
{
public:
	ItemPanelDragAndDropDecorator(CGameEngine* _engine, ItemPanel* _itemPanel);

	void SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped);
	void Update() override;
	void RenderCurrentlyDraggedItem();
	void Render() override;

private:
	int currentDraggedItem;
	int currentDraggedItemOldX;
	int currentDraggedItemOldY;

	std::function<InventoryItemWrapper*(InventoryItemWrapper*)> OnItemDropped;

	void HandleStartedDrag();
	void HandleContinuedDrag();
	void HandleDrop();
};