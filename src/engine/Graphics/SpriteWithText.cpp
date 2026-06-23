#include "SpriteWithText.hpp"



SpriteWithText::SpriteWithText(sf::Texture const & _texture)
	: sprite(_texture)
{
	allTextsOnSprite.clear();
}



int SpriteWithText::AddText(const sf::String &_str)
{
	sf::Text text{SfmlCompat::defaultFont()};
	text.setString(_str);
	text.setFillColor(sf::Color::Black);
	text.setPosition(sf::Vector2f(static_cast<float>(sprite.GetRect().position.x), static_cast<float>(sprite.GetRect().position.y)));

	allTextsOnSprite.push_back(text);

	return allTextsOnSprite.size() - 1;
}


void SpriteWithText::SetTextFont(int _textID, sf::Font const & _font)
{
	allTextsOnSprite[_textID].setFont(_font);
}


void SpriteWithText::SetTextCharacterSize(int _textID, int _characterSize)
{
	allTextsOnSprite[_textID].setCharacterSize(_characterSize);
}


void SpriteWithText::SetTextColor(int _textID, sf::Color const & _color)
{
	allTextsOnSprite[_textID].setFillColor(_color);
}


void SpriteWithText::SetTextStyle(int _textID, sf::Text::Style _style)
{
	allTextsOnSprite[_textID].setStyle(_style);
}



void SpriteWithText::SetTextPos(int _textID, int _xPos, int _yPos)
{
	allTextsOnSprite[_textID].setPosition(sf::Vector2f(static_cast<float>(sprite.GetRect().position.x + _xPos), static_cast<float>(sprite.GetRect().position.y + _yPos)));
}


void SpriteWithText::SetTextPosXCentered(int _textID)
{
	allTextsOnSprite[_textID].setPosition(sf::Vector2f(sprite.GetRect().position.x + sprite.GetRect().size.x/2 - allTextsOnSprite[_textID].getGlobalBounds().size.x/2, allTextsOnSprite[_textID].getGlobalBounds().position.y));
}



void SpriteWithText::SetTextPosYCentered(int _textID)
{
	allTextsOnSprite[_textID].setPosition(sf::Vector2f(allTextsOnSprite[_textID].getGlobalBounds().position.x, sprite.GetRect().position.y + sprite.GetRect().size.y/2 - allTextsOnSprite[_textID].getGlobalBounds().size.y/2));
}


void SpriteWithText::SetTextPosCentered(int _textID)
{
	SetTextPosXCentered(_textID);
	SetTextPosYCentered(_textID);
}

void SpriteWithText::MoveText(int _textID, int _x, int _y)
{
	allTextsOnSprite[_textID].setPosition(sf::Vector2f(allTextsOnSprite[_textID].getGlobalBounds().position.x + _x, allTextsOnSprite[_textID].getGlobalBounds().position.y + _y));
}

void SpriteWithText::ChangeString(int _textID, const sf::String &_str)
{
	allTextsOnSprite[_textID].setString(_str);
}


void SpriteWithText::SetPos(int _x, int _y)
{
	//get the current position of the sprite
	auto xSprite = sprite.GetRect().position.x;
	auto ySprite = sprite.GetRect().position.y;

	//change the position of the sprite
	sprite.SetPos(_x, _y);

	//change the position of the texts
	for (auto &t : allTextsOnSprite)
		t.setPosition(sf::Vector2f(_x + (t.getPosition().x - xSprite), _y + (t.getPosition().y - ySprite)));
}


void SpriteWithText::Scale(float _x, float _y)
{
	//scale the sprite
	sprite.SetScale(_x, _y);

	//scale the texts
	for (auto &t : allTextsOnSprite)
		t.setCharacterSize(static_cast<int>(t.getCharacterSize() * _x));
}


void SpriteWithText::Render(sf::RenderTarget & _target, sf::IntRect * _clip)
{
	//renders the sprite
	sprite.Render(_target, _clip);

	//renders all texts
	for (auto &t : allTextsOnSprite)
		_target.draw(t);
}
