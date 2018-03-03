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
}



void Bar::SetPos(int _x, int _y)
{
	m_frame.SetPos(_x, _y);
	m_bar.SetPos(_x, _y);
}


void Bar::SetPos(float _x, float _y)
{
	m_frame.SetPos(_x, _y);
	m_bar.SetPos(_x, _y);
}




void Bar::Render(sf::RenderTarget &_target, bool _withNumbers)
{
	if (m_pValue != nullptr && m_pMaxValue != nullptr)
	{
		//renders the beam
		m_bar.Render(_target, (float)(*m_pValue) / (float)(*m_pMaxValue), false);
		m_frame.Render(_target);

		if (_withNumbers)
		{
			std::stringstream stream("");
			stream.str("");

			stream << *m_pValue << "/" << *m_pMaxValue;
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