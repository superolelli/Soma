#include "presentation/sfml/app/EconomyApplication.hpp"

#include "content/battle/BattleContentLoader.hpp"
#include "content/economy/EconomyContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "game/CampaignState.hpp"
#include "game/EconomyService.hpp"
#include "presentation/sfml/app/BattleLaunchOptions.hpp"
#include "presentation/sfml/app/ContentDiagnostics.hpp"
#include "presentation/sfml/economy/EconomyTextures.hpp"
#include "presentation/sfml/economy/InventoryPanel.hpp"
#include "presentation/sfml/economy/SkillPanel.hpp"
#include "presentation/sfml/economy/ShopPanel.hpp"
#include "presentation/sfml/economy/RuleBuilderPanel.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>
#include <vector>

namespace soma::presentation
{
    namespace
    {
        bool enterRepository(const char* executablePath)
        {
            const auto repositoryRoot = findSomaRepositoryRoot(executablePath);
            if (!repositoryRoot.has_value())
            {
                std::cerr << "Could not locate the Soma Data directory.\n";
                return false;
            }
            std::filesystem::current_path(*repositoryRoot);
            return true;
        }

        sf::RenderWindow makeEconomyWindow(const content::EconomyContent& content, const std::string& title)
        {
            const auto reference = content.presentation.referenceSize;
            sf::RenderWindow window{sf::VideoMode::getDesktopMode(), title, sf::State::Fullscreen};
            window.setView(sf::View{sf::FloatRect{{0.0F, 0.0F}, {static_cast<float>(reference.x), static_cast<float>(reference.y)}}});
            window.setVerticalSyncEnabled(true);
            return window;
        }

        void addPreviewInventory(game::EconomyService& economy, const content::EconomyContent& content)
        {
            std::vector<const gameplay::ItemDefinition*> equipment;
            for (const auto& [id, item] : content.items)
            {
                static_cast<void>(id);
                if (item.category == gameplay::ItemCategory::equipment)
                {
                    equipment.push_back(&item);
                }
            }
            std::ranges::sort(equipment, {}, [](const auto* item) { return item->id; });
            const std::array colors{gameplay::ItemColor{128, 0, 0}, gameplay::ItemColor{0, 128, 0}, gameplay::ItemColor{0, 0, 128},
                                    gameplay::ItemColor{128, 128, 0}, gameplay::ItemColor{}};
            for (std::size_t index = 0; index < std::min<std::size_t>(equipment.size(), 12); ++index)
            {
                static_cast<void>(economy.grant(*equipment.at(index), 1, colors.at(index % colors.size())));
            }
        }
    } // namespace

    int runSkillApplication(const char* executablePath)
    {
        if (!enterRepository(executablePath))
        {
            return 1;
        }
        const auto battle = content::BattleContentLoader{}.load("Data/content/v1");
        const auto economyContent = content::EconomyContentLoader{}.load("Data/content/v1");
        if (!battle.valid())
        {
            std::cerr << formatContentDiagnostics("Battle content could not be loaded.", battle.diagnostics) << '\n';
            return 1;
        }
        if (!economyContent.valid())
        {
            std::cerr << formatContentDiagnostics("Economy content could not be loaded.", economyContent.diagnostics) << '\n';
            return 1;
        }

        const auto& content = economyContent.content;
        auto window = makeEconomyWindow(content, "Soma - Fähigkeiten");

        const EconomyTextures textures{content, battle.content};
        const sf::Font interfaceFont{content.assets.at(gameplay::AssetId{"font.interface"})};
        const sf::Font titleFont{content.assets.at(gameplay::AssetId{"font.title"})};
        const sf::Font priceFont{content.assets.at(gameplay::AssetId{"font.price"})};
        const sf::Font abilityFont{content.assets.at(gameplay::AssetId{"font.ability"})};
        const EconomyPanelFonts fonts{interfaceFont, titleFont, abilityFont, priceFont};
        game::CampaignState state;
        game::EconomyService economy{state};
        static_cast<void>(economy.grant(content.items.at(gameplay::ItemId{"currency.dice"}), 30));
        SkillPanel panel{window, battle.content, content, state, economy, textures, fonts};

        while (window.isOpen())
        {
            panel.processEvents();
            panel.render();
        }
        return 0;
    }

    int runInventoryApplication(const char* executablePath)
    {
        if (!enterRepository(executablePath))
        {
            return 1;
        }
        const auto loaded = content::EconomyContentLoader{}.load("Data/content/v1");
        if (!loaded.valid())
        {
            std::cerr << formatContentDiagnostics("Economy content could not be loaded.", loaded.diagnostics) << '\n';
            return 1;
        }
        const auto& content = loaded.content;
        const auto battleLoaded = content::BattleContentLoader{}.load("Data/content/v1");
        if (!battleLoaded.valid())
        {
            std::cerr << formatContentDiagnostics("Battle content could not be loaded.", battleLoaded.diagnostics) << '\n';
            return 1;
        }
        auto window = makeEconomyWindow(content, "Soma - Inventar");
        const EconomyTextures textures{content};
        const sf::Font interfaceFont{content.assets.at(gameplay::AssetId{"font.interface"})};
        const sf::Font titleFont{content.assets.at(gameplay::AssetId{"font.title"})};
        const sf::Font abilityFont{content.assets.at(gameplay::AssetId{"font.ability"})};
        const sf::Font priceFont{content.assets.at(gameplay::AssetId{"font.price"})};
        const EconomyPanelFonts fonts{interfaceFont, titleFont, abilityFont, priceFont};
        game::CampaignState state;
        game::EconomyService economy{state};
        addPreviewInventory(economy, content);
        InventoryPanel panel{window, battleLoaded.content, content, state, economy, textures, fonts};
        while (window.isOpen())
        {
            panel.processEvents();
            panel.render();
        }
        return 0;
    }

    int runShopApplication(const char* executablePath)
    {
        if (!enterRepository(executablePath))
        {
            return 1;
        }
        const auto loaded = content::EconomyContentLoader{}.load("Data/content/v1");
        const auto rules = content::RuleContentLoader{}.load("Data/content/v1");
        if (!loaded.valid())
        {
            std::cerr << formatContentDiagnostics("Economy content could not be loaded.", loaded.diagnostics) << '\n';
            return 1;
        }
        if (!rules.valid())
        {
            std::cerr << formatContentDiagnostics("Rule content could not be loaded.", rules.diagnostics) << '\n';
            return 1;
        }
        const auto& content = loaded.content;
        auto window = makeEconomyWindow(content, "Soma - Shop");
        const EconomyTextures textures{content};
        const sf::Font interfaceFont{content.assets.at(gameplay::AssetId{"font.interface"})};
        const sf::Font titleFont{content.assets.at(gameplay::AssetId{"font.title"})};
        const sf::Font abilityFont{content.assets.at(gameplay::AssetId{"font.ability"})};
        const sf::Font priceFont{content.assets.at(gameplay::AssetId{"font.price"})};
        const EconomyPanelFonts fonts{interfaceFont, titleFont, abilityFont, priceFont};
        game::CampaignState state;
        game::EconomyService economy{state};
        game::RuleBuilderService ruleService{state, rules.content};
        for (const auto& [id, definition] : rules.content.fragments)
        {
            static_cast<void>(definition);
            static_cast<void>(ruleService.grant(id));
        }
        addPreviewInventory(economy, content);
        const auto consumable = std::ranges::find_if(content.items, [](const auto& entry) {
            return entry.second.category == gameplay::ItemCategory::consumable;
        });
        if (consumable != content.items.end())
        {
            static_cast<void>(economy.grant(consumable->second, 3));
        }
        ShopPanel panel{window, content, rules.content, state, economy, ruleService, textures, fonts};
        while (window.isOpen())
        {
            panel.processEvents();
            panel.render();
        }
        return 0;
    }

    int runRuleApplication(const char* executablePath)
    {
        if (!enterRepository(executablePath)) return 1;
        const auto economy = content::EconomyContentLoader{}.load("Data/content/v1");
        const auto rules = content::RuleContentLoader{}.load("Data/content/v1");
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
        auto window = makeEconomyWindow(economy.content, "Soma - Regelwerk");
        const sf::Font font{economy.content.assets.at(gameplay::AssetId{"font.interface"})};
        game::CampaignState state;
        game::RuleBuilderService service{state, rules.content};
        for (const auto& [id, definition] : rules.content.fragments)
        {
            static_cast<void>(definition);
            static_cast<void>(service.grant(id, 2));
        }
        const sf::Font titleFont{economy.content.assets.at(gameplay::AssetId{"font.title"})};
        RuleBuilderPanel panel{window, rules.content, state, font, titleFont};
        while (window.isOpen() && !panel.closeRequested())
        {
            panel.processEvents();
            panel.render();
        }
        return 0;
    }
} // namespace soma::presentation
