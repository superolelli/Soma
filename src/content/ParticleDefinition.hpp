#pragma once

#include "content/EconomyContent.hpp"

namespace soma::content
{
    enum class ParticleLayer
    {
        behindCharacters,
        foreground
    };

    struct ParticleRange
    {
        double minimum{};
        double maximum{};
    };

    /// Describes one continuous ambient particle emitter used by a campaign's level presentation.
    struct ParticleEmitterDefinition
    {
        gameplay::AssetId texture;
        ParticleLayer layer{ParticleLayer::behindCharacters};
        int atlasColumns{1};
        int atlasRows{1};
        int maximumParticles{32};
        double emissionRate{4.0};
        ParticleRange lifetimeSeconds{3.0, 5.0};
        Rectangle spawnArea;
        ParticleRange velocityX;
        ParticleRange velocityY;
        double accelerationX{};
        double accelerationY{};
        ParticleRange scale{1.0, 1.0};
        ParticleRange angularVelocity;
        double oscillationAmplitude{};
        double oscillationFrequency{1.0};
        int opacity{255};
    };
} // namespace soma::content
