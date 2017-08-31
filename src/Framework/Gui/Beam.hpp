#pragma once


#include "../Graphics/Sprite.hpp"
#include <sstream>
#include <memory>

class CBeam
{
public:
	CBeam();
	~CBeam();

	//Textures the sprite
	void Load(sf::Texture const &_beamTexture, sf::Texture const &_frameTexture, std::shared_ptr<int> _value, std::shared_ptr<int> _maxValue);

	//Sets the Position for float values
	void SetPos(float _x, float _y);

	//Sets the Position for int values
	void SetPos(int _x, int _y);

	//sets the font
	void SetText(sf::Font const &_font, sf::Color const &_color, int _size);

	//sets the character size
	void SetCharacterSize(int _size){m_text.setCharacterSize(_size);}

	//Renders the beam
	void Render(sf::RenderTarget &_target, bool _withNumbers = false);


private:

	CSprite m_beam;                   //the beam sprite
	CSprite m_frame;                //the frame sprite

	std::weak_ptr<int> m_pValue;                    //the current value
	std::weak_ptr<int> m_pMaxValue;                //the maximum of the value

	sf::Text m_text;                    //the text


};
