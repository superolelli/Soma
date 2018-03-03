#pragma once

#include "../Graphics/Sprite.hpp"
#include <sstream>
#include <memory>

class Bar
{
public:
	Bar();
	~Bar();

	//Textures the sprite
	void Load(sf::Texture const &_beamTexture, sf::Texture const &_frameTexture, int *_value, int *_maxValue);

	//Sets the Position for float values
	void SetPos(float _x, float _y);

	//Sets the Position for int values
	void SetPos(int _x, int _y);

	sf::IntRect const &GetRect() { return m_frame.GetRect(); }

	//sets the font
	void SetText(sf::Font const &_font, sf::Color const &_color, int _size);

	//sets the character size
	void SetCharacterSize(int _size) { m_text.setCharacterSize(_size); }

	//Renders the beam
	void Render(sf::RenderTarget &_target, bool _withNumbers = false);


private:

	CSprite m_bar;                   //the beam sprite
	CSprite m_frame;                //the frame sprite

	int *m_pValue;                    //the current value
	int *m_pMaxValue;                //the maximum of the value

	sf::Text m_text;                    //the text


};