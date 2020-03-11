#include "ItemPanelDecorator.hpp"

void ItemPanelDecorator::Init(CGameEngine *_engine, ItemPanel *_itemPanel)
{
	engine = _engine;
	itemPanel = _itemPanel;
}

void ItemPanelDecorator::Quit()
{
	itemPanel->Clear();
	SAFE_DELETE(itemPanel);
}

void ItemPanelDecorator::Clear()
{
	itemPanel->Clear();
}


sf::IntRect const &ItemPanelDecorator::GetRect() 
{ 
	return itemPanel->GetRect(); 
}


void ItemPanelDecorator::AddItem(Item _item) 
{ 
	itemPanel->AddItem(_item); 
}


void ItemPanelDecorator::SetPos(int _x, int _y) 
{
	itemPanel->SetPos(_x, _y); 
}