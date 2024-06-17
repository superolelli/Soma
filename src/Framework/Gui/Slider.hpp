#pragma once

#include "../Graphics/Sprite.hpp"
#include "../Gameengine.hpp"

class Slider {
public:

	Slider(sf::Texture const& _barTexture, sf::Texture const& _knobTexture);

	void SetMinimum(int _minimum);
	void SetMaximum(int _maximum);
	void SetStepsize(int _stepsize);

	void SetFont(sf::Font &_font);

	void ShowText(bool _showText);

	void SetPos(int _x, int _y);

	void Update(CGameEngine &_engine);
	void Render(sf::RenderTarget &_target);

	sf::IntRect const &GetRect() const { return sliderBar.GetRect(); }

	int GetCurrentValue() { return currentValue; };

private:

	bool knobActive;
	bool showText;

	CSprite knob;
	CSprite sliderBar;

	sf::Text amountText;

	int minimum;
	int maximum;
	int stepsize;

	int currentValue;

};