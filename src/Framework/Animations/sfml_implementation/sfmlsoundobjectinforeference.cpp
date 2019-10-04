#include "sfmlsoundobjectinforeference.h"

namespace SpriterEngine
{

	SfmlSoundObjectInfoReference::SfmlSoundObjectInfoReference(sf::SoundBuffer &buffer)
	{
		sound.setBuffer(buffer);
		sound.setRelativeToListener(true);
	}

	void SfmlSoundObjectInfoReference::playTrigger()
	{
		if (getTriggerCount())
		{
			sound.play();
		}
	}

	void SfmlSoundObjectInfoReference::setSoundPosition(float _x, float _y)
	{
		sound.setPosition(_x, 0, _y);
	}

	void SfmlSoundObjectInfoReference::setSoundAttenuation(float _attenuation)
	{
		sound.setAttenuation(_attenuation);
	}

	void SfmlSoundObjectInfoReference::setSoundMinDistance(float _distance)
	{
		sound.setMinDistance(_distance);
	}

	void SfmlSoundObjectInfoReference::enableSpatialSounds()
	{
		sound.setRelativeToListener(false);
	}

	void SfmlSoundObjectInfoReference::setPlaybackVolume()
	{
		sound.setVolume(100 * getVolume());
	}

	void SfmlSoundObjectInfoReference::setPlaybackPanning()
	{
		sound.setPosition(100 * getPanning(), 0, 0);
	}

}
