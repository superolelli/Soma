#include "presentation/sfml/app/BattleApplication.hpp"

#include "content/battle/BattleContentLoader.hpp"
#include "content/economy/EconomyContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "game/CampaignBattleContentBuilder.hpp"
#include "game/CampaignState.hpp"
#include "presentation/sfml/app/BattleLaunchOptions.hpp"
#include "presentation/sfml/app/ContentDiagnostics.hpp"
#include "presentation/sfml/battle/BattleScreenSession.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>

namespace soma::presentation
{
    namespace
    {
        bool containsOnlyKnownEnemies(const content::BattleContent& content, const std::vector<gameplay::ContentDefinitionId>& enemies)
        {
            for (const auto& enemy : enemies)
            {
                if (!content.combatants.contains(enemy) || content.combatants.at(enemy).team != gameplay::Team::enemies)
                {
                    std::cerr << "Unknown enemy: " << enemy.value() << '\n';
                    return false;
                }
            }
            return true;
        }

    } // namespace

    int runBattleApplication(int argc, char** argv)
    {
        const auto repositoryRoot = findSomaRepositoryRoot(argv[0]);
        if (!repositoryRoot)
        {
            std::cerr << "Could not locate the Soma Data directory.\n";
            return 1;
        }
        std::filesystem::current_path(*repositoryRoot);
        const auto loaded = content::BattleContentLoader{}.load("Data/content/v1");
        const auto economyLoaded = content::EconomyContentLoader{}.load("Data/content/v1");
        const auto rulesLoaded = content::RuleContentLoader{}.load("Data/content/v1");
        if (!loaded.valid())
        {
            std::cerr << formatContentDiagnostics("Battle content could not be loaded.", loaded.diagnostics) << '\n';
            return 1;
        }
        if (!economyLoaded.valid())
        {
            std::cerr << formatContentDiagnostics("Economy content could not be loaded.", economyLoaded.diagnostics) << '\n';
            return 1;
        }
        if (!rulesLoaded.valid())
        {
            std::cerr << formatContentDiagnostics("Rule content could not be loaded.", rulesLoaded.diagnostics) << '\n';
            return 1;
        }

        game::CampaignState campaignState;
        const auto campaignContent = game::CampaignBattleContentBuilder{}.build(loaded.content, economyLoaded.content, campaignState);
        const auto launchOptions = parseBattleLaunchOptions(argc, argv);
        if (launchOptions.error)
        {
            std::cerr << *launchOptions.error << '\n';
            return 2;
        }
        const auto encounterFound = campaignContent.encounters.find(launchOptions.encounter);
        if (encounterFound == campaignContent.encounters.end())
        {
            std::cerr << "Unknown encounter: " << launchOptions.encounter.value() << '\n';
            return 2;
        }
        if (!containsOnlyKnownEnemies(campaignContent, launchOptions.enemies))
        {
            return 2;
        }

        sf::RenderWindow window{sf::VideoMode::getDesktopMode(), "Soma", sf::State::Fullscreen};
        window.setVerticalSyncEnabled(true);
        const auto& encounter = encounterFound->second;
        BattleScreenRequest request;
        request.encounter = encounter;
        request.enemies = launchOptions.enemies;
        request.background = encounter.background;
        request.music = encounter.music;
        request.randomSeed = launchOptions.randomSeed.value_or(std::random_device{}());
        request.developmentLaunchOptions = launchOptions;
        request.returnWhenResolved = launchOptions.exitOnFinish;
        const auto result = runBattleScreenSession(
            window, campaignContent, economyLoaded.content, rulesLoaded.content, campaignState, std::move(request));
        static_cast<void>(result);
        return 0;
    }
} // namespace soma::presentation
