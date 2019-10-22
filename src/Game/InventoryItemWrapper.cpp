#include "InventoryItemWrapper.hpp"

void InventoryItemWrapper::Init(Item && _item, CSprite && _sprite)
{
	item = std::move(_item);
	sprite = std::move(_sprite);
}

void InventoryItemWrapper::Render(sf::RenderTarget & _target)
{
	sprite.Render(_target);
}

void InventoryItemWrapper::SetPos(int _x, int _y)
{
	sprite.SetPos(_x, _y);
}

sf::IntRect & InventoryItemWrapper::GetGlobalBounds()
{
	return sprite.GetGlobalRect();
}

Item & InventoryItemWrapper::GetItem()
{
	return item;
}

CSprite & InventoryItemWrapper::GetSprite()
{
	return sprite;
}

bool InventoryItemWrapper::Contains(const sf::Vector2i &_point)
{
	return sprite.GetGlobalRect().contains(_point);
}
