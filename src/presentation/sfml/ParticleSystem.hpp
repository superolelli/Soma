#pragma once

#include "content/ParticleDefinition.hpp"

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <filesystem>
#include <random>
#include <span>
#include <unordered_map>
#include <vector>

namespace soma::presentation
{
    struct ParticleEmitterConfig
    {
        content::ParticleEmitterDefinition definition;
        std::filesystem::path texture;
    };

    [[nodiscard]] std::vector<ParticleEmitterConfig> resolveParticleEmitters(
        std::span<const content::ParticleEmitterDefinition> definitions, const std::unordered_map<gameplay::AssetId, std::filesystem::path>& assets);

    /// Simulates and renders data-driven ambient sprite particles for exploration and battle scenes.
    class ParticleSystem
    {
      public:
        explicit ParticleSystem(std::vector<ParticleEmitterConfig> emitters, std::uint64_t randomSeed);

        void update(sf::Time elapsed, const sf::FloatRect& viewport);
        void draw(sf::RenderTarget& target, content::ParticleLayer layer) const;
        [[nodiscard]] std::size_t particleCount(content::ParticleLayer layer) const noexcept;

      private:
        struct Emitter
        {
            content::ParticleEmitterDefinition definition;
            sf::Texture texture;
            double emissionAccumulator{};
        };

        struct Particle
        {
            std::size_t emitter{};
            sf::Vector2f position;
            sf::Vector2f velocity;
            float rotation{};
            float angularVelocity{};
            float scale{1.0F};
            float age{};
            float lifetime{1.0F};
            float oscillationPhase{};
            int atlasFrame{};
        };

        void prewarm(const sf::FloatRect& viewport);
        void spawn(std::size_t emitter, const sf::FloatRect& viewport, float initialAge = 0.0F);
        void advance(Particle& particle, float seconds) const;
        [[nodiscard]] float random(const content::ParticleRange& range);
        [[nodiscard]] float random(float minimum, float maximum);
        [[nodiscard]] int random(int minimum, int maximum);
        [[nodiscard]] std::size_t countForEmitter(std::size_t emitter) const noexcept;

        std::vector<Emitter> emitters_;
        std::vector<Particle> particles_;
        std::mt19937_64 random_;
        bool initialized_{};
    };
} // namespace soma::presentation
