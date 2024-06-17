#pragma once

#include "ItemPanel.hpp"

class ItemPanelDecorator
{
public:

	ItemPanelDecorator(CGameEngine* _engine, ItemPanel* _itemPanel);
	~ItemPanelDecorator();

	virtual void Update() = 0;
	virtual void Render() = 0;

	void Clear();
	void AddItem(Item _item);
	void SetPos(int _x, int _y);

	sf::IntRect const &GetRect();

	void SortItemsAccordingToColor();
	void SortItemsAccordingToNames();

protected:
	CGameEngine *engine;
	ItemPanel *itemPanel;
};