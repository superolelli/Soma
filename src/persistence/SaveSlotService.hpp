#pragma once

#include "persistence/SaveRepository.hpp"

#include <array>
#include <optional>

namespace soma::persistence
{
    enum class SaveSlotState
    {
        empty,
        occupied,
        invalid
    };

    struct SaveSlotSummary
    {
        int index{};
        SaveSlotState state{SaveSlotState::empty};
        std::string playerName;
        std::string error;
    };

    /// Provides the three main-menu save slots and creates or loads their save files.
    class SaveSlotService
    {
      public:
        static constexpr int slotCount = 3;

        SaveSlotService(std::filesystem::path directory, const SaveRepository& repository);

        [[nodiscard]] std::array<SaveSlotSummary, slotCount> discover() const;
        [[nodiscard]] gameplay::Result<SaveGame> load(int index) const;
        [[nodiscard]] gameplay::Result<SaveGame> create(int index, std::string playerName, bool overwrite = false) const;
        [[nodiscard]] gameplay::Result<std::filesystem::path> store(int index, const SaveGame& saveGame) const;
        [[nodiscard]] gameplay::Result<int> remove(int index) const;
        [[nodiscard]] std::filesystem::path pathFor(int index) const;

      private:
        [[nodiscard]] static bool validIndex(int index) noexcept;

        std::filesystem::path directory_;
        const SaveRepository& repository_;
    };
} // namespace soma::persistence
