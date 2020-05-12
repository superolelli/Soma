#pragma once

#include "ItemPanel.hpp"

class ItemPanelDecorator
{
public:
	virtual void Init(CGameEngine *_engine, ItemPanel *_itemPanel);
	virtual void Update() = 0;
	virtual void Render() = 0;

	void Quit();
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