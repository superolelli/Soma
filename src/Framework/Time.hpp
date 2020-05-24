#pragma once


#include "../Framework/Patterns/singleton.hpp"
#include "SFML\System.hpp"



#define g_pTimer CTime::Get()

class CTime : public TSingleton<CTime>
{
public:

	//starts the clock
	CTime();

	//Updates the clock
	void Update();

	void ResetTimeSinceLastUpdate();

	//gives the time since the start of the game
	sf::Time const &GetCurrentTime(){ return m_current; }

	//gives the time since the last frame
	sf::Time const &GetElapsedTime(){return m_elapsed;}

	double GetElapsedTimeAsMilliseconds(){ return static_cast<double>(m_elapsed.asMicroseconds()) / 1000.0; }
	double GetElapsedTimeAsSeconds() { return static_cast<double>(m_elapsed.asMicroseconds()) / 1000000.0; }

	double GetElapsedTimeSinceLastUpdateAsSeconds() { return static_cast<double>(m_elapsedSinceUpdate.asMicroseconds()) / 1000000.0; }


private:

	sf::Clock m_clock;
	sf::Time m_current;
	sf::Time m_elapsed;
	sf::Time m_elapsedSinceUpdate;
};



