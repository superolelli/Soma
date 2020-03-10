#include "Slider.hpp"

Slider::Slider()
{
	stepsize = 1;
	minimum = 1;
	maximum = 100;
	currentValue = 1;
	knobActive = false;
	showText = false;

	amountText.setString(std::to_string(currentValue));
	amountText.setCharacterSize(18);
}

void Slider::Load(sf::Texture const & _barTexture, sf::Texture const &_knobTexture)
{
	knob.Load(_knobTexture);
	sliderBar.Load(_barTexture);
}


void Slider::SetFont(sf::Font &_font)
{
	amountText.setFont(_font);
}


void Slider::ShowText(bool _showText)
{
	showText = _showText;
}

void Slider::SetMinimum(int _minimum)
{
	minimum = _minimum;
	currentValue = minimum;
	amountText.setString(std::to_string(currentValue));
	knob.SetPos(sliderBar.GetRect().left, sliderBar.GetRect().top + sliderBar.GetRect().height / 2 - knob.GetRect().height / 2);
}

void Slider::SetMaximum(int _maximum)
{
	maximum = _maximum;
}

void Slider::SetStepsize(int _stepsize)
{
	stepsize = _stepsize;
}

void Slider::SetPos(int _x, int _y)
{
	sliderBar.SetPos(_x, _y);
	knob.SetPos(_x, _y + sliderBar.GetRect().height / 2 - knob.GetRect().height / 2);
	amountText.setPosition(_x + sliderBar.GetRect().width + 10, _y);
}

void Slider::Update(CGameEngine & _engine)
{
	if (_engine.GetButtonstates(ButtonID::Left) == Pressed && knob.GetRect().contains(_engine.GetMousePos()))
	{
		knobActive = true;
	}
	else if (_engine.GetButtonstates(ButtonID::Left) == Held && knobActive)
	{
		int xPos = std::min(_engine.GetMousePos().x, sliderBar.GetRect().left + sliderBar.GetRect().width - knob.GetRect().width);
		xPos = std::max(sliderBar.GetRect().left, xPos);

		float sizeOfOneStep = float(sliderBar.GetRect().width - knob.GetRect().width) / float((maximum - minimum) / stepsize);
		currentValue = minimum + std::round(float(xPos - sliderBar.GetRect().left) / sizeOfOneStep) * stepsize;
		amountText.setString(std::to_string(currentValue));
		int nearestStep = float(currentValue - minimum) * sizeOfOneStep + sliderBar.GetRect().left;
		knob.SetPos(nearestStep, knob.GetRect().top);
	}
	else if (_engine.GetButtonstates(ButtonID::Left) == Released && knobActive)
	{
		knobActive = false;
	}
}

void Slider::Render(sf::RenderTarget & _target)
{
	sliderBar.Render(_target);
	knob.Render(_target);

	if (showText)
		_target.draw(amountText);
}
