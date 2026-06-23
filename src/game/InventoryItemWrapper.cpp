#include "InventoryItemWrapper.hpp"

InventoryItemWrapper::InventoryItemWrapper(Item && _item, CSprite && _sprite)
	: item(_item)
	, sprite(_sprite)
{
	colorRectangle.setFillColor(_item.color);
	colorRectangle.setSize(sf::Vector2f(97, 97));

	numberText.setFillColor(sf::Color::Black);
	numberText.setOutlineColor(sf::Color::White);
	numberText.setOutlineThickness(2.0f);
	numberText.setCharacterSize(13);
	numberText.setFont(g_pFonts->f_kingArthur);
	numberText.setString(std::to_string(item.number));
}

void InventoryItemWrapper::Render(sf::RenderTarget & _target)
{
	_target.draw(colorRectangle);
	sprite.Render(_target);

	if ((item.id < EQUIPMENT_ITEMS_START || item.id >= CONSUMABLE_ITEMS_START) && item.number > 0)
		_target.draw(numberText);
}

void InventoryItemWrapper::SetPos(int _x, int _y)
{
	sprite.SetPos(_x, _y);
	colorRectangle.setPosition(sf::Vector2f(_x + 2, _y + 2));
	numberText.setPosition(sf::Vector2f(_x + sprite.GetGlobalRect().size.x / 2 - numberText.getLocalBounds().size.x / 2, _y + 72));
}

void InventoryItemWrapper::SetCenterPos(int _x, int _y)
{
	sprite.SetPos(_x - sprite.GetGlobalRect().size.x / 2, _y - sprite.GetGlobalRect().size.y / 2);
	colorRectangle.setPosition(sf::Vector2f(sprite.GetGlobalRect().position.x + 2, sprite.GetGlobalRect().position.y + 2));
	numberText.setPosition(sf::Vector2f(sprite.GetGlobalRect().position.x + sprite.GetGlobalRect().size.x / 2 - numberText.getLocalBounds().size.x / 2, sprite.GetGlobalRect().position.y + 60));
}

sf::IntRect InventoryItemWrapper::GetGlobalBounds()
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

void InventoryItemWrapper::SetSpriteColor(int r, int g, int b)
{
	sprite.SetColor(r, g, b);
}

void InventoryItemWrapper::SetItemAmount(int _amount)
{
	item.number = _amount;
	numberText.setString(std::to_string(item.number));
}

bool InventoryItemWrapper::Contains(const sf::Vector2i &_point)
{
	return sprite.GetGlobalRect().contains(_point);
}
