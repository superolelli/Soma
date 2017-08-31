#include "Beam.hpp"



CBeam::CBeam()
{
}


CBeam::~CBeam()
{

}




void CBeam::Load(sf::Texture const &_beamTexture, sf::Texture const &_frameTexture, std::shared_ptr<int> _value, std::shared_ptr<int> _maxValue)
{
	//Set the texture
	m_beam.Load(_beamTexture, 2, _beamTexture.getSize().x / 2, _beamTexture.getSize().y);

	m_frame.Load(_frameTexture);

	m_pValue = _value;
	m_pMaxValue = _maxValue;

	m_text.setCharacterSize(12);
	m_text.setFillColor(sf::Color::Black);
}



void CBeam::SetPos(int _x, int _y)
{
	m_frame.SetPos(_x, _y);
	m_beam.SetPos(_x, _y);
}


void CBeam::SetPos(float _x, float _y)
{
	m_frame.SetPos(_x, _y);
	m_beam.SetPos(_x, _y);
}




void CBeam::Render(sf::RenderTarget &_target, bool _withNumbers)
{
	//check if the the pointers point to something
	auto val = m_pValue.lock();
	auto mVal = m_pMaxValue.lock();

	if (val != nullptr && mVal != nullptr)
	{
		//renders the beam
		m_beam.Render(_target, (float)(*val) / (float)(*mVal), false);
		m_frame.Render(_target);

		if (_withNumbers)
		{
			std::stringstream stream("");
			stream.str("");

			stream << *val << "/" << *mVal;
			m_text.setString(stream.str());
			m_text.setPosition(m_beam.GetRect().width / 2 + m_beam.GetRect().left - m_text.getGlobalBounds().width / 2, m_beam.GetRect().height / 2 + m_beam.GetRect().top - m_text.getGlobalBounds().height / 2 - 2);
			_target.draw(m_text);
		}
	}
}



void CBeam::SetText(sf::Font const &_font, sf::Color const &_color, int _size)
{
	m_text.setFillColor(_color);
	m_text.setFont(_font);
	m_text.setCharacterSize(_size);
}