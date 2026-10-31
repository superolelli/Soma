#include "sfmlsoundobjectinforeference.h"

#include "presentation/sfml/common/AudioSettings.hpp"

namespace SpriterEngine
{

    SfmlSoundObjectInfoReference::SfmlSoundObjectInfoReference(sf::SoundBuffer& buffer)
        : sound(buffer)
    {
        sound.setRelativeToListener(true);
    }

    void SfmlSoundObjectInfoReference::playTrigger()
    {
        if (getTriggerCount())
        {
            setPlaybackVolume();
            sound.play();
        }
    }

    void SfmlSoundObjectInfoReference::setSoundPosition(float _x, float _y)
    {
        sound.setPosition({_x, 0.f, _y});
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
        sound.setVolume(soma::presentation::AudioSettings::sound(100.0F * getVolume()));
    }

    void SfmlSoundObjectInfoReference::setPlaybackPanning()
    {
        sound.setPosition({static_cast<float>(100 * getPanning()), 0, 0});
    }

} // namespace SpriterEngine
