#pragma once

#include "content/WorldContent.hpp"

#include <optional>

namespace soma::simulation
{
    struct GeneratedRoom
    {
        gameplay::AssetId background;
        std::optional<gameplay::EncounterId> encounter;
        std::optional<gameplay::LootableId> lootable;
        bool boss{};
        bool encounterPending{};
        bool lootableOpened{};

        auto operator<=>(const GeneratedRoom&) const = default;
    };

    struct GeneratedParallax
    {
        std::vector<gameplay::AssetId> secondLayerTiles;
        std::vector<gameplay::AssetId> thirdLayerTiles;
        gameplay::AssetId fourthLayer;
        int secondLayerDivisor{3};
        int thirdLayerDivisor{6};
        int fourthLayerDivisor{9};

        auto operator<=>(const GeneratedParallax&) const = default;
    };

    /// Tracks the generated rooms and mutable traversal progress of one active level.
    class LevelRun
    {
      public:
        LevelRun(gameplay::LevelId level, std::vector<GeneratedRoom> rooms, GeneratedParallax parallax = {});

        [[nodiscard]] const gameplay::LevelId& level() const noexcept;
        [[nodiscard]] const std::vector<GeneratedRoom>& rooms() const noexcept;
        [[nodiscard]] std::size_t currentRoom() const noexcept;
        [[nodiscard]] bool finished() const noexcept;
        [[nodiscard]] const GeneratedRoom& room() const;
        [[nodiscard]] const GeneratedParallax& parallax() const noexcept;

        bool enterRoom(std::size_t index);
        [[nodiscard]] std::optional<gameplay::EncounterId> takePendingEncounter();
        bool openLootable();
        bool openLootable(std::size_t roomIndex);
        bool restoreProgress(std::size_t currentRoom);

      private:
        gameplay::LevelId level_;
        std::vector<GeneratedRoom> rooms_;
        GeneratedParallax parallax_;
        std::size_t currentRoom_{};
    };
} // namespace soma::simulation
