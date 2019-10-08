#pragma once

#include <SFML/Audio.hpp>
#include "../../Framework/Patterns/singleton.hpp"

enum soundID {DOOR, CLICK, SKILL_ACQUIRED};

#define g_pSounds SoundManager::Get()
class SoundManager : public TSingleton<SoundManager>
{
public:
	void LoadSounds();
	void PlaySound(soundID _id);
	void Update();

	void Quit();

private:

	sf::SoundBuffer door;
	sf::SoundBuffer click;
	sf::SoundBuffer skillAcquired;

	std::vector<sf::Sound*> freeSoundObjects;
	std::vector<sf::Sound*> usedSoundObjects;
};