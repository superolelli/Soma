#ifndef SOUNDOBJECTINFOREFERENCE_H
#define SOUNDOBJECTINFOREFERENCE_H

#include "../objectinfo/universalobjectinterface.h"

namespace SpriterEngine
{

	class FileReference;

	class SoundObjectInfoReference : public UniversalObjectInterface
	{
	public:
		SoundObjectInfoReference();

		int getTriggerCount() override;

		real getVolume() override;
		real getPanning() override;

		void setTriggerCount(int newTriggerCount) override;

		void setVolume(real newVolume) override;
		void setPanning(real newPanning) override;

		/* MY CHANGES - START */
		void setSoundPosition(float _x, float _y) override;
		void setSoundAttenuation(float _attenuation) override;
		void setSoundMinDistance(float _distance) override;
		void enableSpatialSounds() override;
		/* MY CHANGES -END */

		// TODO: override playTrigger() to play back sound using getTriggerCount() to determine whether to play;

		void setToBlendedLinear(UniversalObjectInterface *aObject, UniversalObjectInterface *bObject, real t, real blendRatio, ObjectRefInstance *blendedRefInstance = 0) override;

	private:
		virtual void setPlaybackVolume();
		virtual void setPlaybackPanning();

		real volume;
		real panning;

		int triggerCount;
	};

}

#endif // SOUNDOBJECTINFOREFERENCE_H
