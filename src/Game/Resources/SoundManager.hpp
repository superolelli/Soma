#pragma once

#include <SFML/Audio.hpp>
#include "../../Framework/Patterns/singleton.hpp"

enum soundID {DOOR, CLICK, SKILL_ACQUIRED, INVENTORY_DROP, LOOTABLE_TAKE_ALL, BARREL, CABINET, CHEST, CUP, KEG, RUBBISH, SACK, SKELETON, STONEPILE, TABLE, WAGON, NUMBER_OF_SOUNDS};

const int LOOTABLE_SOUNDS_START = 5;

#define g_pSounds SoundManager::Get()
class SoundManager : public TSingleton<SoundManager>
{
public:
	void LoadSounds();
	void PlaySound(soundID _id);
	void Update();

	void Quit();

private:

	sf::SoundBuffer soundBuffers[NUMBER_OF_SOUNDS];
	std::vector<sf::Sound*> freeSoundObjects;
	std::vector<sf::Sound*> usedSoundObjects;
};