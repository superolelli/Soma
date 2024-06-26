#pragma once

#include "../Graphics/Sprite.hpp"
#include <sstream>
#include <memory>

class Bar
{
public:
	Bar(sf::Texture const& _beamTexture, sf::Texture const& _frameTexture, int* _value, int* _maxValue);
	~Bar();

	//Sets the Position for float values
	void SetPos(float _x, float _y);

	//Sets the Position for int values
	void SetPos(int _x, int _y);

	void SetOffsetForInnerPart(int _x, int _y);

	void SetValuePtr(int *_value);
	void SetMaxValuePtr(int *_maxValue);

	sf::IntRect const &GetRect() { return m_frame.GetRect(); }

	void SetSmoothTransformationTime(double _time) { baseAnimationTime = _time; currentAnimationTime = _time; }

	//sets the font
	void SetText(sf::Font const &_font, sf::Color const &_color, int _size);

	//sets the character size
	void SetCharacterSize(int _size) { m_text.setCharacterSize(_size); }

	// returns the change value
	int Update(double _elapsedTime);

	//Renders the beam
	void Render(sf::RenderTarget &_target, bool _withNumbers = false);


private:

	CSprite m_bar;                   //the beam sprite
	CSprite m_frame;                //the frame sprite

	int *m_pValue;                    //the current value
	int *m_pMaxValue;                //the maximum of the value

	int xOffset;
	int yOffset;

	float valueToRender;

	int lastValue;

	int smoothAnimationBaseValue;
	double baseAnimationTime;
	double currentAnimationTime;

	sf::Text m_text;                    //the text

	void StartAnimation();
	void HandleAnimation(double _elapsedTime);

};