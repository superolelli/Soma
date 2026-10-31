#include "presentation/sfml/common/ParticleSystem.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace soma::presentation
{
    std::vector<ParticleEmitterConfig> resolveParticleEmitters(std::span<const content::ParticleEmitterDefinition> definitions,
                                                               const std::unordered_map<gameplay::AssetId, std::filesystem::path>& assets)
    {
        std::vector<ParticleEmitterConfig> result;
        result.reserve(definitions.size());
        for (const auto& definition : definitions)
        {
            result.push_back({definition, assets.at(definition.texture)});
        }
        return result;
    }

    ParticleSystem::ParticleSystem(std::vector<ParticleEmitterConfig> emitters, std::uint64_t randomSeed)
        : random_(randomSeed)
    {
        emitters_.reserve(emitters.size());
        for (auto& emitter : emitters)
        {
            sf::Texture texture{emitter.texture};
            texture.setSmooth(true);
            emitters_.push_back({std::move(emitter.definition), std::move(texture)});
        }
    }

    void ParticleSystem::update(sf::Time elapsed, const sf::FloatRect& viewport)
    {
        if (!initialized_)
        {
            initialized_ = true;
            prewarm(viewport);
        }

        const auto seconds = std::min(elapsed.asSeconds(), 0.1F);
        for (auto& particle : particles_)
        {
            advance(particle, seconds);
        }
        std::erase_if(particles_, [](const auto& particle) {
            return particle.age >= particle.lifetime;
        });

        for (std::size_t index = 0; index < emitters_.size(); ++index)
        {
            auto& emitter = emitters_.at(index);
            emitter.emissionAccumulator += emitter.definition.emissionRate * static_cast<double>(seconds);
            while (emitter.emissionAccumulator >= 1.0 && countForEmitter(index) < static_cast<std::size_t>(emitter.definition.maximumParticles))
            {
                spawn(index, viewport);
                emitter.emissionAccumulator -= 1.0;
            }
        }
    }

    void ParticleSystem::draw(sf::RenderTarget& target, content::ParticleLayer layer) const
    {
        for (const auto& particle : particles_)
        {
            const auto& emitter = emitters_.at(particle.emitter);
            const auto& definition = emitter.definition;
            if (definition.layer != layer)
            {
                continue;
            }

            const auto textureSize = emitter.texture.getSize();
            const auto frameWidth = static_cast<int>(textureSize.x / static_cast<unsigned int>(definition.atlasColumns));
            const auto frameHeight = static_cast<int>(textureSize.y / static_cast<unsigned int>(definition.atlasRows));
            const auto column = particle.atlasFrame % definition.atlasColumns;
            const auto row = particle.atlasFrame / definition.atlasColumns;
            sf::Sprite sprite{emitter.texture};
            sprite.setTextureRect({{column * frameWidth, row * frameHeight}, {frameWidth, frameHeight}});
            sprite.setOrigin({static_cast<float>(frameWidth) / 2.0F, static_cast<float>(frameHeight) / 2.0F});
            const auto oscillation = static_cast<float>(definition.oscillationAmplitude) *
                                     std::sin(particle.oscillationPhase + particle.age * static_cast<float>(definition.oscillationFrequency));
            sprite.setPosition(particle.position + sf::Vector2f{oscillation, 0.0F});
            sprite.setRotation(sf::degrees(particle.rotation));
            sprite.setScale({particle.scale, particle.scale});

            const auto fadeIn = std::min(1.0F, particle.age / std::min(0.35F, particle.lifetime * 0.2F));
            const auto fadeOut = std::min(1.0F, (particle.lifetime - particle.age) / std::min(0.6F, particle.lifetime * 0.3F));
            const auto alpha = static_cast<std::uint8_t>(std::clamp(fadeIn * fadeOut, 0.0F, 1.0F) * static_cast<float>(definition.opacity));
            sprite.setColor(sf::Color{255, 255, 255, alpha});
            target.draw(sprite);
        }
    }

    std::size_t ParticleSystem::particleCount(content::ParticleLayer layer) const noexcept
    {
        return static_cast<std::size_t>(std::ranges::count_if(particles_, [this, layer](const auto& particle) {
            return emitters_.at(particle.emitter).definition.layer == layer;
        }));
    }

    void ParticleSystem::prewarm(const sf::FloatRect& viewport)
    {
        for (std::size_t index = 0; index < emitters_.size(); ++index)
        {
            const auto& definition = emitters_.at(index).definition;
            const auto averageLifetime = (definition.lifetimeSeconds.minimum + definition.lifetimeSeconds.maximum) / 2.0;
            const auto desired = std::min(definition.maximumParticles, static_cast<int>(std::round(definition.emissionRate * averageLifetime)));
            for (auto particle = 0; particle < desired; ++particle)
            {
                const auto lifetime = random(definition.lifetimeSeconds);
                spawn(index, viewport, random(0.0F, lifetime));
            }
        }
    }

    void ParticleSystem::spawn(std::size_t emitterIndex, const sf::FloatRect& viewport, float initialAge)
    {
        const auto& definition = emitters_.at(emitterIndex).definition;
        const auto left = viewport.position.x + static_cast<float>(definition.spawnArea.x);
        const auto top = viewport.position.y + static_cast<float>(definition.spawnArea.y);
        Particle particle;
        particle.emitter = emitterIndex;
        particle.position = {random(left, left + static_cast<float>(definition.spawnArea.width)),
                             random(top, top + static_cast<float>(definition.spawnArea.height))};
        particle.velocity = {random(definition.velocityX), random(definition.velocityY)};
        particle.rotation = random(0.0F, 360.0F);
        particle.angularVelocity = random(definition.angularVelocity);
        particle.scale = random(definition.scale);
        particle.lifetime = random(definition.lifetimeSeconds);
        particle.oscillationPhase = random(0.0F, 2.0F * std::numbers::pi_v<float>);
        particle.atlasFrame = random(0, definition.atlasColumns * definition.atlasRows - 1);
        if (initialAge > 0.0F)
        {
            advance(particle, std::min(initialAge, particle.lifetime * 0.95F));
        }
        particles_.push_back(particle);
    }

    void ParticleSystem::advance(Particle& particle, float seconds) const
    {
        const auto& definition = emitters_.at(particle.emitter).definition;
        particle.velocity += sf::Vector2f{static_cast<float>(definition.accelerationX), static_cast<float>(definition.accelerationY)} * seconds;
        particle.position += particle.velocity * seconds;
        particle.rotation += particle.angularVelocity * seconds;
        particle.age += seconds;
    }

    float ParticleSystem::random(const content::ParticleRange& range)
    {
        return random(static_cast<float>(range.minimum), static_cast<float>(range.maximum));
    }

    float ParticleSystem::random(float minimum, float maximum)
    {
        return std::uniform_real_distribution<float>{minimum, maximum}(random_);
    }

    int ParticleSystem::random(int minimum, int maximum)
    {
        return std::uniform_int_distribution<int>{minimum, maximum}(random_);
    }

    std::size_t ParticleSystem::countForEmitter(std::size_t emitter) const noexcept
    {
        return static_cast<std::size_t>(std::ranges::count(particles_, emitter, &Particle::emitter));
    }
} // namespace soma::presentation
