#include "presentation/sfml/BattleApplication.hpp"
#include "presentation/sfml/EconomyApplication.hpp"
#include "presentation/sfml/ExplorationApplication.hpp"
#include "presentation/sfml/GameApplication.hpp"

#include <filesystem>
#include <iostream>
#include <string_view>

int main(int argc, char** argv)
{
    if (argc == 1 || (argc > 1 && std::string_view{argv[1]} == "--menu"))
    {
        return soma::presentation::runGameApplication(argv[0]);
    }
    if (argc > 1 && (std::string_view{argv[1]} == "--savegame" || std::string_view{argv[1]} == "--save-game"))
    {
        if (argc != 3)
        {
            std::cerr << "Usage: Soma.exe --savegame <path>\n";
            return 1;
        }
        return soma::presentation::runGameApplication(argv[0], std::filesystem::absolute(argv[2]));
    }
    if (argc > 1 && std::string_view{argv[1]} == "--skills")
    {
        return soma::presentation::runSkillApplication(argv[0]);
    }
    if (argc > 1 && std::string_view{argv[1]} == "--inventory")
    {
        return soma::presentation::runInventoryApplication(argv[0]);
    }
    if (argc > 1 && std::string_view{argv[1]} == "--shop")
    {
        return soma::presentation::runShopApplication(argv[0]);
    }
    if (argc > 1 && std::string_view{argv[1]} == "--rules")
    {
        return soma::presentation::runRuleApplication(argv[0]);
    }
    if (argc > 1 && std::string_view{argv[1]} == "--explore")
    {
        return soma::presentation::runExplorationApplication(argc, argv);
    }
    return soma::presentation::runBattleApplication(argc, argv);
}
