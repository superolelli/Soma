#pragma once

#include "content/economy/EconomyContent.hpp"
#include "content/world/WorldContent.hpp"
#include "content/rules/RuleContent.hpp"
#include "gameplay/common/Result.hpp"
#include "persistence/SaveGame.hpp"

#include <filesystem>

namespace soma::persistence
{
    /// Serializes, validates, and atomically replaces complete game save files.
    class SaveRepository
    {
      public:
        SaveRepository(const content::EconomyContent& economy, const content::WorldContent& world, const content::RuleContent& rules);

        [[nodiscard]] gameplay::Result<SaveGame> load(const std::filesystem::path& path) const;
        [[nodiscard]] gameplay::Result<std::filesystem::path> save(const std::filesystem::path& path,
                                                                   const SaveGame& saveGame) const;

      private:
        [[nodiscard]] gameplay::Result<SaveGame> validate(SaveGame saveGame) const;

        const content::EconomyContent& economy_;
        const content::WorldContent& world_;
        const content::RuleContent& rules_;
    };
} // namespace soma::persistence
