#include "presentation/sfml/app/ExplorationApplication.hpp"

#include "content/battle/BattleContentLoader.hpp"
#include "content/economy/EconomyContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "content/world/WorldContentLoader.hpp"
#include "game/CampaignState.hpp"
#include "game/AuthoredLevelLoader.hpp"
#include "game/EconomyService.hpp"
#include "presentation/sfml/app/BattleLaunchOptions.hpp"
#include "presentation/sfml/app/ContentDiagnostics.hpp"
#include "presentation/sfml/exploration/ExplorationSession.hpp"

#include <filesystem>
#include <iostream>
#include <random>
#include <string>

namespace soma::presentation
{
    namespace
    {
        void grantDebugConsumables(const content::EconomyContent& content, game::EconomyService& service)
        {
            for (const auto& [id, definition] : content.items)
            {
                static_cast<void>(id);
                if (definition.category == gameplay::ItemCategory::consumable)
                {
                    static_cast<void>(service.grant(definition, 2));
                }
            }
        }
    }

    int runExplorationApplication(int argc, char** argv)
    {
        const auto repository = findSomaRepositoryRoot(argv[0]);
        if (!repository)
        {
            std::cerr << "Could not locate the Soma Data directory.\n";
            return 1;
        }
        std::filesystem::current_path(*repository);
        const auto battle = content::BattleContentLoader{}.load("Data/content/v1");
        const auto economy = content::EconomyContentLoader{}.load("Data/content/v1");
        const auto rules = content::RuleContentLoader{}.load("Data/content/v1");
        if (!battle.valid())
        {
            std::cerr << formatContentDiagnostics("Battle content could not be loaded.", battle.diagnostics) << '\n';
            return 1;
        }
        if (!economy.valid())
        {
            std::cerr << formatContentDiagnostics("Economy content could not be loaded.", economy.diagnostics) << '\n';
            return 1;
        }
        if (!rules.valid())
        {
            std::cerr << formatContentDiagnostics("Rule content could not be loaded.", rules.diagnostics) << '\n';
            return 1;
        }
        const auto world = content::WorldContentLoader{}.load("Data/content/v1", battle.content, economy.content);
        if (!world.valid())
        {
            std::cerr << formatContentDiagnostics("World content could not be loaded.", world.diagnostics) << '\n';
            return 1;
        }
        auto level = gameplay::LevelId{"level.bang.1"};
        std::uint64_t randomSeed = std::random_device{}();
        std::optional<std::filesystem::path> authoredLevelPath;
        for (auto index = 2; index < argc; ++index)
        {
            const auto argument = std::string{argv[index]};
            if (argument == "--seed")
            {
                if (index + 1 >= argc)
                {
                    std::cerr << "Missing value for --seed\n";
                    return 2;
                }
                const auto value = std::string_view{argv[++index]};
                const auto parsedSeed = parseRandomSeed(value);
                if (!parsedSeed)
                {
                    std::cerr << "Invalid --seed value: " << value << '\n';
                    return 2;
                }
                randomSeed = *parsedSeed;
            }
            else if (argument == "--level-layout" && index + 1 < argc)
            {
                authoredLevelPath = argv[++index];
            }
            else if (!argument.starts_with("--"))
            {
                level = gameplay::LevelId{argument};
            }
        }
        if (!world.content.levels.contains(level))
        {
            std::cerr << "Unknown level: " << level.value() << '\n';
            return 2;
        }

        game::CampaignState campaignState;
        game::EconomyService economyService{campaignState};
        grantDebugConsumables(economy.content, economyService);
        std::optional<simulation::LevelRun> authoredLevel;
        try
        {
            if (authoredLevelPath)
            {
                authoredLevel = game::loadAuthoredLevel(*authoredLevelPath, level, world.content);
            }
        }
        catch (const std::exception& exception)
        {
            std::cerr << "Could not load authored level '" << authoredLevelPath->string() << "': " << exception.what() << '\n';
            return 2;
        }
        return runExplorationSession(
            battle.content, economy.content, world.content, rules.content, level, campaignState, economyService, randomSeed,
            std::move(authoredLevel));
    }
} // namespace soma::presentation
