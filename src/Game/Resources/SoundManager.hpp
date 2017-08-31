#pragma once

#include <SFML/Audio.hpp>
#include "../../Framework/Patterns/singleton.hpp"


#define g_pSounds CSoundManager::Get()
class CSoundManager : public TSingleton<CSoundManager>
{
public:
	//Loads all sounds
	void LoadSounds();

};