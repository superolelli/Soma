#include "ScrollableItemPanel.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"
#include "Resources/ObjectPropertiesManager.hpp"

void ScrollableItemPanel::Init(CGameEngine * _engine)
{
	ItemPanel::Init(_engine);

	currentUpperRow = 0;
	itemPanel.Load(g_pTextures->scrollableItemPanel);

	scrollbar.Init(35, 566);
	scrollbar.SetColor(sf::Color(28, 28, 28));
}

void ScrollableItemPanel::SetPos(int _x, int _y)
{
	itemPanel.SetPos(_x, _y);
	scrollbar.SetPos(itemPanel.GetGlobalRect().left + 600, itemPanel.GetGlobalRect().top + 12);
}


void ScrollableItemPanel::Update()
{
	UpdateScrollbar();
}


void ScrollableItemPanel::UpdateScrollbar()
{
	scrollbar.Update(*engine);

	if (scrollbar.GetCurrentStep() != currentUpperRow)
	{
		currentUpperRow = scrollbar.GetCurrentStep();
		RecalculatePositionsOfItems();
	}
}


void ScrollableItemPanel::RecalculatePositionsOfItems()
{
	int yPos = itemPanel.GetGlobalRect().top + 11;
	int currentIndex = 0;

	for (int y = 0; y < 5; y++)
	{
		int xPos = itemPanel.GetGlobalRect().left + 11;
		for (int x = 0; x < 5; x++)
		{
			currentIndex = (currentUpperRow + y) * 5 + x;
			if (currentIndex < items.size() && items[currentIndex] != nullptr)
				items[currentIndex]->SetPos(xPos, yPos);

			xPos += 116;
		}
		yPos += 116;
	}
}


void ScrollableItemPanel::Render(int _excludeItemNumber)
{
	itemPanel.Render(engine->GetRenderTarget());
	scrollbar.Render(engine->GetRenderTarget());
	RenderItems(currentUpperRow * 5, currentUpperRow * 5 + 25, _excludeItemNumber);
}



void ScrollableItemPanel::AddItem(Item _item)
{
	InventoryItemWrapper *newItem = new InventoryItemWrapper;

	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);
	newItem->Init(std::move(_item), std::move(newSprite));

	int freeSlot = GetFirstFreeSlot();
	if (freeSlot == -1)
		freeSlot = items.size();

	int xPos = (freeSlot % 5) * 116 + itemPanel.GetGlobalRect().left + 11;
	int yPos = (freeSlot / 5) * 116 + itemPanel.GetGlobalRect().top + 11 - currentUpperRow * 116;
	newItem->SetPos(xPos, yPos);

	if (freeSlot < items.size())
		items[freeSlot] = newItem;
	else
		items.push_back(newItem);

	scrollbar.SetNumberOfSteps(std::max(1, (int)(items.size() - 21) / 5 + 1));
}


void ScrollableItemPanel::SortItemsAccordingToNames()
{
	items.erase(std::remove_if(items.begin(), items.end(), [](InventoryItemWrapper* item) {return item == nullptr;}), items.end());

	std::sort(items.begin(), items.end(), [&](InventoryItemWrapper* item1, InventoryItemWrapper* item2) {
		return g_pObjectProperties->getItemStats(item1->GetItem().id).name < g_pObjectProperties->getItemStats(item2->GetItem().id).name;});

	currentUpperRow = 0;
	RecalculatePositionsOfItems();
}


void ScrollableItemPanel::SortItemsAccordingToColor()
{
	items.erase(std::remove_if(items.begin(), items.end(), [](InventoryItemWrapper* item) {return item == nullptr;}), items.end());

	std::sort(items.begin(), items.end(), [](InventoryItemWrapper* item1, InventoryItemWrapper* item2) {
		return item1->GetItem().color.toInteger() < item2->GetItem().color.toInteger();});

	currentUpperRow = 0;
	RecalculatePositionsOfItems();
}