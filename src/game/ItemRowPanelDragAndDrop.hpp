#pragma once

#include "ItemRowPanel.hpp"

class ItemRowPanelDragAndDrop : public ItemRowPanel
{
public:

	void Init(CGameEngine *_engine) override;
	void SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped);
	void Update() override;
	void RenderCurrentlyDraggedItem();

private:

	int currentDraggedItem;
	int currentDraggedItemOldX;
	int currentDraggedItemOldY;

	std::function<InventoryItemWrapper*(InventoryItemWrapper*)> OnItemDropped;

	void HandleStartedDrag();
	void HandleContinuedDrag();
	void HandleDrop();

	void RenderItems() override;
};