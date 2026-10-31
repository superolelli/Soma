#include "presentation/sfml/app/BattleLaunchOptions.hpp"

#include <algorithm>
#include <charconv>
#include <string>

namespace soma::presentation
{
    std::optional<std::uint64_t> parseRandomSeed(std::string_view value) noexcept
    {
        std::uint64_t result{};
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
        if (error != std::errc{} || end != value.data() + value.size())
        {
            return std::nullopt;
        }
        return result;
    }

    BattleLaunchOptions parseBattleLaunchOptions(int argc, char** argv)
    {
        BattleLaunchOptions options;
        for (int index = 1; index < argc; ++index)
        {
            const auto argument = std::string{argv[index]};
            if (argument == "--auto")
            {
                options.automatic = true;
                continue;
            }
            if (argument == "--exit-on-finish")
            {
                options.exitOnFinish = true;
                continue;
            }
            if (argument == "--auto-ability" && index + 1 < argc)
            {
                options.automatic = true;
                options.automaticAbility = gameplay::AbilityId{argv[++index]};
                continue;
            }
            if (argument == "--encounter" && index + 1 < argc)
            {
                options.encounter = gameplay::ContentDefinitionId{argv[++index]};
                continue;
            }
            if (argument == "--seed")
            {
                if (index + 1 >= argc)
                {
                    options.error = "Missing value for --seed";
                    return options;
                }
                const auto value = std::string_view{argv[++index]};
                options.randomSeed = parseRandomSeed(value);
                if (!options.randomSeed)
                {
                    options.error = "Invalid --seed value: " + std::string{value};
                    return options;
                }
                continue;
            }
            if (argument != "--enemies")
            {
                continue;
            }
            while (++index < argc && options.enemies.size() < 4)
            {
                auto id = std::string{argv[index]};
                if (id.starts_with("--"))
                {
                    --index;
                    break;
                }
                for (auto escaped = id.find("\\_"); escaped != std::string::npos; escaped = id.find("\\_"))
                {
                    id.replace(escaped, 2, "_");
                }
                options.enemies.emplace_back(std::move(id));
            }
        }
        return options;
    }

    std::optional<std::filesystem::path> findSomaRepositoryRoot(const char* executable)
    {
        std::vector<std::filesystem::path> candidates{std::filesystem::current_path(), std::filesystem::absolute(executable).parent_path()};
        for (auto candidate : candidates)
        {
            for (int level = 0; level < 8; ++level)
            {
                if (std::filesystem::exists(candidate / "Data/content/v1") && std::filesystem::exists(candidate / "Data/Sprites"))
                {
                    return std::filesystem::canonical(candidate);
                }
                const auto parent = candidate.parent_path();
                if (parent == candidate)
                {
                    break;
                }
                candidate = parent;
            }
        }
        return std::nullopt;
    }

} // namespace soma::presentation
