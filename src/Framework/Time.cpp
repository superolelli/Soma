#include "Time.hpp"



CTime::CTime()
{
	m_elapsed = sf::seconds(0);
	m_elapsedSinceUpdate = sf::seconds(0);
	m_current = sf::seconds(0);
	m_clock.restart();
}




void CTime::Update()
{
	m_current += m_clock.getElapsedTime();
	m_elapsed = m_clock.restart();
	m_elapsedSinceUpdate += m_elapsed;
}

void CTime::ResetTimeSinceLastUpdate()
{
	m_elapsedSinceUpdate = sf::seconds(0);
}