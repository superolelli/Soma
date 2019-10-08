#include "SoundManager.hpp"


void SoundManager::LoadSounds()
{
	door.loadFromFile("Data/Sound/door.ogg");
	click.loadFromFile("Data/Sound/click.wav");
	skillAcquired.loadFromFile("Data/Sound/skill_acquired.wav");

	for (int i = 0; i < 10; i++)
		freeSoundObjects.push_back(new sf::Sound);
}


void SoundManager::Quit()
{
	for (auto &s : freeSoundObjects)
		delete(s);

	for (auto &s : usedSoundObjects)
		delete(s);
}

void SoundManager::Update()
{
	for (auto it = usedSoundObjects.begin(); it != usedSoundObjects.end();)
	{
		if ((*it)->getStatus() == sf::SoundSource::Stopped)
		{
			freeSoundObjects.push_back(*it);
			it = usedSoundObjects.erase(it);
			continue;
		}
		it++;
	}
}

void SoundManager::PlaySound(soundID _id)
{
	if (!freeSoundObjects.empty())
	{
		usedSoundObjects.push_back(freeSoundObjects.back());
		freeSoundObjects.pop_back();

		switch (_id)
		{
		case DOOR:
			usedSoundObjects.back()->setBuffer(door);
			break;
		case CLICK:
			usedSoundObjects.back()->setBuffer(click);
			break;
		case SKILL_ACQUIRED:
			usedSoundObjects.back()->setBuffer(skillAcquired);
			break;
		}

		usedSoundObjects.back()->play();
	}
}