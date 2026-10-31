#pragma once

#include "gameplay/common/StrongId.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace soma::presentation
{
    struct BattleLaunchOptions
    {
        gameplay::ContentDefinitionId encounter{"encounter.bang.default"};
        std::vector<gameplay::ContentDefinitionId> enemies;
        bool automatic{};
        bool exitOnFinish{};
        std::optional<gameplay::AbilityId> automaticAbility;
        std::optional<std::uint64_t> randomSeed;
        std::optional<std::string> error;
    };

    [[nodiscard]] std::optional<std::uint64_t> parseRandomSeed(std::string_view value) noexcept;
    [[nodiscard]] BattleLaunchOptions parseBattleLaunchOptions(int argc, char** argv);
    [[nodiscard]] std::optional<std::filesystem::path> findSomaRepositoryRoot(const char* executable);
} // namespace soma::presentation
