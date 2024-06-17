#pragma once


#include "Sprite.hpp"

class SpriteWithText
{
public:

	SpriteWithText(sf::Texture const& _texture);

	//adds text to the sprite and returns the index
	int AddText(const sf::String &_str);

	void SetTextFont(int _textID, sf::Font const &_font);

	void SetTextCharacterSize(int _textID, int _characterSize);

	void SetTextColor(int _textID, sf::Color const &_color);

	void SetTextStyle(int _textID, sf::Text::Style _style);

	void SetTextPos(int _textID, int _xPos, int _yPos);

	void SetTextPosXCentered(int _textID);
	void SetTextPosYCentered(int _textID);
	void SetTextPosCentered(int _textID);

	void SetPos(int _x, int _y);

	void Scale(float _x, float _y);

	void ChangeString(int _textID, const sf::String &_str);

	void Render(sf::RenderTarget &_target, sf::IntRect *_clip = nullptr);

	sf::IntRect const &GetRect() { return sprite.GetRect(); }

	void MoveText(int _textID, int _x, int _y);

private:

	CSprite sprite;

	//all the texts on the sprite
	std::vector<sf::Text> allTextsOnSprite;
};