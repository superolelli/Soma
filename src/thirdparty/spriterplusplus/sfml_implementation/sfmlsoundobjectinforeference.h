#ifndef SFMLSOUNDOBJECTINFOREFERENCE_H
#define SFMLSOUNDOBJECTINFOREFERENCE_H

#include "../override/soundobjectinforeference.h"

#include "SFML/Audio.hpp"

namespace SpriterEngine
{
	

	class SfmlSoundObjectInfoReference : public SoundObjectInfoReference
	{
	public:
		SfmlSoundObjectInfoReference(sf::SoundBuffer &buffer);

		void playTrigger() override;

		/* MY CHANGES - START */
		void setSoundPosition(float _x, float _y) override;
		void setSoundAttenuation(float _attenuation) override;
		void setSoundMinDistance(float _distance) override;
		void enableSpatialSounds() override;
		/* MY CHANGES -END */

	private:
		void setPlaybackVolume() override;
		void setPlaybackPanning() override;

		sf::Sound sound;
	};

}

#endif // SFMLSOUNDOBJECTINFOREFERENCE_H
