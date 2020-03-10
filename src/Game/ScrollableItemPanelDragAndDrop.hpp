#pragma once

#include "ScrollableItemPanel.hpp"


class ScrollableItemPanelDragAndDrop : public ScrollableItemPanel
{
public:

	void Init(CGameEngine *_engine) override;
	void Update() override;
	void RenderCurrentlyDraggedItem();

	void SetOnItemDroppedCallback(std::function<InventoryItemWrapper*(InventoryItemWrapper*)> _onItemDropped);

private:

	std::function<InventoryItemWrapper*(InventoryItemWrapper*)> OnItemDropped;

	int currentDraggedItemOldX;
	int currentDraggedItemOldY;
	int currentDraggedItem;

	void HandleDragAndDrop();
	void HandleStartedDrag();
	void HandleContinuedDrag();
	void HandleDrop();

	void RenderItems() override;
};