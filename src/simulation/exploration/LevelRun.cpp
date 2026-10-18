#include "simulation/exploration/LevelRun.hpp"

#include <algorithm>
#include <stdexcept>

namespace soma::simulation
{
    LevelRun::LevelRun(gameplay::LevelId level, std::vector<GeneratedRoom> rooms, GeneratedParallax parallax)
        : level_(std::move(level)), rooms_(std::move(rooms)), parallax_(std::move(parallax))
    {
        if (rooms_.size() < 2 || !rooms_.back().boss)
        {
            throw std::invalid_argument("A level run requires regular rooms followed by a boss room");
        }
    }

    const gameplay::LevelId& LevelRun::level() const noexcept { return level_; }
    const std::vector<GeneratedRoom>& LevelRun::rooms() const noexcept { return rooms_; }
    std::size_t LevelRun::currentRoom() const noexcept { return currentRoom_; }
    bool LevelRun::finished() const noexcept { return currentRoom_ + 1 == rooms_.size() && !rooms_.back().encounterPending; }
    const GeneratedRoom& LevelRun::room() const { return rooms_.at(currentRoom_); }
    const GeneratedParallax& LevelRun::parallax() const noexcept { return parallax_; }

    bool LevelRun::enterRoom(std::size_t index)
    {
        if (index >= rooms_.size() || index < currentRoom_ || index > currentRoom_ + 1)
        {
            return false;
        }
        currentRoom_ = index;
        return true;
    }

    std::optional<gameplay::EncounterId> LevelRun::takePendingEncounter()
    {
        auto& current = rooms_.at(currentRoom_);
        if (!current.encounterPending || !current.encounter)
        {
            return std::nullopt;
        }
        current.encounterPending = false;
        return current.encounter;
    }

    bool LevelRun::openLootable()
    {
        return openLootable(currentRoom_);
    }

    bool LevelRun::openLootable(std::size_t roomIndex)
    {
        if (roomIndex >= rooms_.size() || roomIndex < currentRoom_ || roomIndex > currentRoom_ + 1)
        {
            return false;
        }
        auto& room = rooms_.at(roomIndex);
        if (!room.lootable || room.lootableOpened)
        {
            return false;
        }
        room.lootableOpened = true;
        return true;
    }

    bool LevelRun::restoreProgress(std::size_t currentRoom)
    {
        if (currentRoom >= rooms_.size())
        {
            return false;
        }
        currentRoom_ = currentRoom;
        return true;
    }
} // namespace soma::simulation
