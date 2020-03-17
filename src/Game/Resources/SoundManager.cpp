#include "SoundManager.hpp"


void SoundManager::LoadSounds()
{
	soundBuffers[DOOR].loadFromFile("Data/Sound/door.ogg");
	soundBuffers[CLICK].loadFromFile("Data/Sound/click.wav");
	soundBuffers[SKILL_ACQUIRED].loadFromFile("Data/Sound/skill_acquired.wav");
	soundBuffers[INVENTORY_DROP].loadFromFile("Data/Sound/inventory_drop.wav");
	soundBuffers[LOOTABLE_TAKE_ALL].loadFromFile("Data/Sound/lootable_take_all.wav");
	soundBuffers[DRINK].loadFromFile("Data/Sound/drink.wav");
	soundBuffers[DAMAGE_OVER_TIME].loadFromFile("Data/Sound/damage_over_time.wav");
	soundBuffers[CRITICAL_HIT].loadFromFile("Data/Sound/critical_hit.wav");
	soundBuffers[DODGED].loadFromFile("Data/Sound/dodged.wav");
	soundBuffers[BARREL].loadFromFile("Data/Sound/Lootables/barrel.ogg");
	soundBuffers[CABINET].loadFromFile("Data/Sound/Lootables/cabinet.ogg");
	soundBuffers[CHEST].loadFromFile("Data/Sound/Lootables/chest.wav");
	soundBuffers[CUP].loadFromFile("Data/Sound/Lootables/cup.ogg");
	soundBuffers[KEG].loadFromFile("Data/Sound/Lootables/keg.ogg");
	soundBuffers[RUBBISH].loadFromFile("Data/Sound/Lootables/stonepile.ogg");
	soundBuffers[SACK].loadFromFile("Data/Sound/Lootables/sack.ogg");
	soundBuffers[SKELETON].loadFromFile("Data/Sound/Lootables/skeleton.ogg");
	soundBuffers[STONEPILE].loadFromFile("Data/Sound/Lootables/stonepile.ogg");
	soundBuffers[TABLE].loadFromFile("Data/Sound/Lootables/table.ogg");
	soundBuffers[WAGON].loadFromFile("Data/Sound/Lootables/wagon.ogg");

	for (int i = 0; i < 10; i++)
	{
		freeSoundObjects.push_back(new sf::Sound);
		freeSoundObjects[i]->setRelativeToListener(true);
	}
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
		usedSoundObjects.back()->setBuffer(soundBuffers[_id]);
		usedSoundObjects.back()->play();
	}
}