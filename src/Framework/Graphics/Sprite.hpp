#pragma once

#include "SFML\Graphics.hpp"
#include <iostream>

class CSprite
{
public:
	CSprite(sf::Texture const &_texture);

	// for animated sprite
	CSprite(sf::Texture const& _texture, int _numFrames, int _frameWidth, int _frameHeight);

	~CSprite();

	//sets the part of the texture to render
	void SetTextureRect(sf::IntRect const &_rect);

	//sets the texture rect to default
	void SetTextureRectToDefault();

	//Sets the Position for float values
	void SetPos(float _x, float _y);

	//Sets the Position for int values
	void SetPos(int _x, int _y);

	//Moves the sprite(float)
	void Move(float _x, float _y);

	//sets the rotating point
	void SetOrigin(float _x, float _y);

	//rotates the sprite
	void Rotate(float _angle);

	//sets the rotation
	void SetRotation(float _angle);

	//sets the rotation to 0
	void ResetRotation();

	//sets the scale
	void SetScale(float _x, float _y);

	//sets the sprite's colour
	void SetColor(int _r, int _g, int _b, int _a = 255);

	const sf::Color &GetColor();

	//Renders a normal sprite
	void Render(sf::RenderTarget &_target, sf::IntRect *clip = nullptr);

	//Renders an animated sprite (if picturewise is true, the frameNumber is casted to an int)
	void Render(sf::RenderTarget &_target, float _fFrameNumber, bool _picturewise = true);

	//returns the sprite's rectangle
	sf::IntRect const &GetRect() const { return m_Rect; }

	//returns the global rect
	sf::IntRect GetGlobalRect() const;

private:

	sf::Sprite m_Sprite;                   //the sprite
	sf::IntRect m_Rect;                    //the sprite's rect
	sf::IntRect m_FrameRect;               //the rect of an frame
	int m_NumFrames;                   //the number of frames 
	int m_FrameWidth;                  //the width of one frame
	int m_FrameHeight;                 //the height of one frame
	int m_NumFramesX;                  //the number of frames on the x axis
};

