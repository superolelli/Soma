#include "Bar.hpp"



Bar::Bar()
{
}


Bar::~Bar()
{

}


void Bar::Load(sf::Texture const &_beamTexture, sf::Texture const &_frameTexture, int *_value, int *_maxValue)
{
	//Set the texture
	m_bar.Load(_beamTexture, 2, _beamTexture.getSize().x / 2, _beamTexture.getSize().y);

	m_frame.Load(_frameTexture);

	m_pValue = _value;
	m_pMaxValue = _maxValue;

	m_text.setCharacterSize(12);
	m_text.setFillColor(sf::Color::Black);

	baseAnimationTime = 0.0;
	currentAnimationTime = 0.0;

	smoothAnimationBaseValue = 0;

	xOffset = 0;
	yOffset = 0;

	if (_value != nullptr)
	{
		lastValue = *_value;
		valueToRender = (float)*_value;
	}
}

void Bar::SetValuePtr(int *_value)
{
	m_pValue = _value;
	lastValue = *_value;
	currentAnimationTime = baseAnimationTime;
}

void Bar::SetMaxValuePtr(int * _maxValue)
{
	m_pMaxValue = _maxValue;
}

void Bar::SetOffsetForInnerPart(int _x, int _y)
{
	xOffset = _x;
	yOffset = _y;
}

void Bar::SetPos(int _x, int _y)
{
	m_frame.SetPos(_x, _y);
	m_bar.SetPos(_x + xOffset, _y + yOffset);
}


void Bar::SetPos(float _x, float _y)
{
	m_frame.SetPos(_x, _y);
	m_bar.SetPos(_x + xOffset, _y + yOffset);
}




void Bar::Update(double _elapsedTime)
{
	if (baseAnimationTime > 0.0)
	{
		if (lastValue != *m_pValue)
			StartAnimation();

		HandleAnimation(_elapsedTime);
	}
	else
		valueToRender = (float)*m_pValue;
}


void Bar::StartAnimation()
{
	currentAnimationTime = 0.0;
	smoothAnimationBaseValue = lastValue;
	lastValue = *m_pValue;
}


void Bar::HandleAnimation(double _elapsedTime)
{
	if (currentAnimationTime < baseAnimationTime)
	{
		currentAnimationTime += _elapsedTime;
		valueToRender = (double)smoothAnimationBaseValue + (currentAnimationTime / baseAnimationTime) * (double)(*m_pValue - smoothAnimationBaseValue);
	}
	else
		valueToRender = (float)*m_pValue;
}



void Bar::Render(sf::RenderTarget &_target, bool _withNumbers)
{
	if (m_pValue != nullptr && m_pMaxValue != nullptr)
	{
		//renders the beam
		m_bar.Render(_target, valueToRender / (float)(*m_pMaxValue), false);
		m_frame.Render(_target);

		if (_withNumbers)
		{
			std::stringstream stream("");
			stream.str("");

			stream << static_cast<int>(valueToRender) << "/" << *m_pMaxValue;
			m_text.setString(stream.str());
			m_text.setPosition(m_bar.GetRect().width / 2 + m_bar.GetRect().left - m_text.getGlobalBounds().width / 2, m_bar.GetRect().height / 2 + m_bar.GetRect().top - m_text.getGlobalBounds().height / 2 - 2);
			_target.draw(m_text);
		}
	}
}



void Bar::SetText(sf::Font const &_font, sf::Color const &_color, int _size)
{
	m_text.setFillColor(_color);
	m_text.setFont(_font);
	m_text.setCharacterSize(_size);
}