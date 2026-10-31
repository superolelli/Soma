#include "presentation/sfml/app/BattleApplication.hpp"
#include "presentation/sfml/app/EconomyApplication.hpp"
#include "presentation/sfml/app/ExplorationApplication.hpp"
#include "presentation/sfml/app/GameApplication.hpp"

#include <filesystem>
#include <exception>
#include <iostream>
#include <string_view>

namespace
{
    int dispatchApplication(int argc, char** argv)
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
} // namespace

int main(int argc, char** argv)
{
    try
    {
        return dispatchApplication(argc, argv);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Soma could not continue: " << error.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "Soma could not continue because of an unknown error.\n";
        return 1;
    }
}
