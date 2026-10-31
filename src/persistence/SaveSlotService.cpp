#include "persistence/SaveSlotService.hpp"

#include <system_error>

namespace soma::persistence
{
    SaveSlotService::SaveSlotService(std::filesystem::path directory, const SaveRepository& repository)
        : directory_(std::move(directory)), repository_(repository)
    {
    }

    std::array<SaveSlotSummary, SaveSlotService::slotCount> SaveSlotService::discover() const
    {
        std::array<SaveSlotSummary, slotCount> result;
        for (auto index = 0; index < slotCount; ++index)
        {
            auto& slot = result.at(static_cast<std::size_t>(index));
            slot.index = index;
            const auto path = pathFor(index);
            std::error_code probeError;
            const auto exists = std::filesystem::exists(path, probeError);
            if (probeError)
            {
                slot.state = SaveSlotState::invalid;
                slot.error = "Save slot could not be inspected: " + probeError.message();
                continue;
            }
            if (!exists)
            {
                continue;
            }
            const auto loaded = repository_.load(path);
            if (loaded.hasValue())
            {
                slot.state = SaveSlotState::occupied;
                slot.playerName = loaded.value().playerName;
            }
            else
            {
                slot.state = SaveSlotState::invalid;
                slot.error = loaded.error();
            }
        }
        return result;
    }

    gameplay::Result<SaveGame> SaveSlotService::load(int index) const
    {
        if (!validIndex(index))
        {
            return gameplay::Result<SaveGame>::failure("Save slot index is outside 0..2");
        }
        return repository_.load(pathFor(index));
    }

    gameplay::Result<SaveGame> SaveSlotService::create(int index, std::string playerName, bool overwrite) const
    {
        if (!validIndex(index))
        {
            return gameplay::Result<SaveGame>::failure("Save slot index is outside 0..2");
        }
        const auto path = pathFor(index);
        std::error_code probeError;
        const auto exists = std::filesystem::exists(path, probeError);
        if (probeError)
        {
            return gameplay::Result<SaveGame>::failure("Save slot could not be inspected: " + probeError.message());
        }
        if (!overwrite && exists)
        {
            return gameplay::Result<SaveGame>::failure("Save slot is already occupied");
        }
        SaveGame saveGame;
        saveGame.playerName = std::move(playerName);
        const auto saved = repository_.save(path, saveGame);
        if (!saved.hasValue())
        {
            return gameplay::Result<SaveGame>::failure(saved.error());
        }
        return gameplay::Result<SaveGame>::success(std::move(saveGame));
    }

    gameplay::Result<std::filesystem::path> SaveSlotService::store(int index, const SaveGame& saveGame) const
    {
        if (!validIndex(index))
        {
            return gameplay::Result<std::filesystem::path>::failure("Save slot index is outside 0..2");
        }
        return repository_.save(pathFor(index), saveGame);
    }

    gameplay::Result<int> SaveSlotService::remove(int index) const
    {
        if (!validIndex(index))
        {
            return gameplay::Result<int>::failure("Save slot index is outside 0..2");
        }
        std::error_code error;
        const auto removed = std::filesystem::remove(pathFor(index), error);
        if (error)
        {
            return gameplay::Result<int>::failure("Save slot could not be deleted: " + error.message());
        }
        if (!removed)
        {
            return gameplay::Result<int>::failure("Save slot is already empty");
        }
        return gameplay::Result<int>::success(index);
    }

    std::filesystem::path SaveSlotService::pathFor(int index) const
    {
        return directory_ / ("slot-" + std::to_string(index + 1) + ".json");
    }

    bool SaveSlotService::validIndex(int index) noexcept
    {
        return index >= 0 && index < slotCount;
    }
} // namespace soma::persistence
