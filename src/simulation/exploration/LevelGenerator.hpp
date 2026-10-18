#pragma once

#include "gameplay/random/IRandom.hpp"
#include "simulation/exploration/LevelRun.hpp"

namespace soma::simulation
{
    /// Generates a level run that satisfies room, encounter-budget, boss, and parallax specifications.
    class LevelGenerator
    {
      public:
        LevelGenerator(const content::WorldContent& content, gameplay::IRandom& random);
        [[nodiscard]] LevelRun generate(const gameplay::LevelId& level) const;

      private:
        [[nodiscard]] GeneratedParallax generateParallax(const content::LevelDefinition& level) const;
        [[nodiscard]] std::vector<gameplay::EncounterId> generateEncounters(const content::LevelDefinition& level) const;
        [[nodiscard]] std::vector<gameplay::AssetId> generateLayerTiles(const std::vector<gameplay::AssetId>& candidates,
                                                                        std::size_t count) const;

        const content::WorldContent& content_;
        gameplay::IRandom& random_;
    };
} // namespace soma::simulation
