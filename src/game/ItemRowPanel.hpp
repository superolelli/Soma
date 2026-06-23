#pragma once

#include "ItemPanel.hpp"

class ItemRowPanel : public ItemPanel
{
public:

	ItemRowPanel(CGameEngine* _engine);
	void Update() override;
	void Render(int _excludeItemNumber = -1) override;
	void AddItem(Item _item) override;
	void SetPos(int _x, int _y) override;
	void Clear() override;
};