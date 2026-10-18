#pragma once

#include <filesystem>
#include <optional>

namespace soma::presentation
{
    int runGameApplication(const char* executablePath,
                           std::optional<std::filesystem::path> saveGamePath = std::nullopt);
}
