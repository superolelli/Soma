#include "presentation/sfml/app/GameApplication.hpp"

#include "content/battle/BattleContentLoader.hpp"
#include "content/economy/EconomyContentLoader.hpp"
#include "content/flow/GameFlowContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "content/world/WorldContentLoader.hpp"
#include "game/CampaignLifecycleService.hpp"
#include "game/CampaignProgression.hpp"
#include "game/EconomyService.hpp"
#include "persistence/SaveRepository.hpp"
#include "persistence/SaveSlotService.hpp"
#include "presentation/sfml/app/BattleLaunchOptions.hpp"
#include "presentation/sfml/app/ContentDiagnostics.hpp"
#include "presentation/sfml/economy/EconomySoundPlayer.hpp"
#include "presentation/sfml/economy/EconomyTextures.hpp"
#include "presentation/sfml/economy/InventoryPanel.hpp"
#include "presentation/sfml/exploration/ExplorationSession.hpp"
#include "presentation/sfml/app/MainMenuScreen.hpp"
#include "presentation/sfml/app/MainRoomScreen.hpp"
#include "presentation/sfml/economy/ShopPanel.hpp"
#include "presentation/sfml/economy/SkillPanel.hpp"
#include "presentation/sfml/economy/RuleBuilderPanel.hpp"
#include "presentation/sfml/app/StartupScreen.hpp"

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <stdexcept>

namespace soma::presentation
{
    namespace
    {
        struct LoadedGameContent
        {
            content::BattleContent battle;
            content::EconomyContent economy;
            content::WorldContent world;
            content::GameFlowContent flow;
            content::RuleContent rules;
        };

        struct GameContentLoadResult
        {
            std::optional<LoadedGameContent> content;
            std::string error;
        };

        using SaveWriter = std::function<gameplay::Result<std::filesystem::path>(const persistence::SaveGame&)>;

        GameContentLoadResult loadGameContent(content::GameFlowContent flow)
        {
            constexpr auto contentRoot = "Data/content/v1";
            auto battle = content::BattleContentLoader{}.load(contentRoot);
            auto economy = content::EconomyContentLoader{}.load(contentRoot);
            auto rules = content::RuleContentLoader{}.load(contentRoot);
            if (!battle.valid())
            {
                return {{}, formatContentDiagnostics("Battle content could not be loaded.", battle.diagnostics)};
            }
            if (!economy.valid())
            {
                return {{}, formatContentDiagnostics("Economy content could not be loaded.", economy.diagnostics)};
            }
            if (!rules.valid())
            {
                return {{}, formatContentDiagnostics("Rule content could not be loaded.", rules.diagnostics)};
            }
            auto world = content::WorldContentLoader{}.load(contentRoot, battle.content, economy.content);
            if (!world.valid())
            {
                return {{}, formatContentDiagnostics("World content could not be loaded.", world.diagnostics)};
            }
            return {LoadedGameContent{std::move(battle.content), std::move(economy.content), std::move(world.content), std::move(flow),
                                      std::move(rules.content)}, {}};
        }

        sf::RenderWindow makeWindow(const content::GameFlowContent& flow)
        {
            sf::RenderWindow window{sf::VideoMode::getDesktopMode(), "Soma", sf::State::Fullscreen};
            window.setView(sf::View{sf::FloatRect{{0.0F, 0.0F},
                                                 {static_cast<float>(flow.referenceSize.x), static_cast<float>(flow.referenceSize.y)}}});
            window.setVerticalSyncEnabled(true);
            return window;
        }

        /// Owns one loaded save and coordinates the hub, panels, levels, progression, and save boundaries.
        class GameSession
        {
          public:
            GameSession(sf::RenderWindow& window,
                        const LoadedGameContent& content,
                        StartupScreen& statusScreen,
                        persistence::SaveGame save,
                        SaveWriter saveWriter)
                : window_(window)
                , content_(content)
                , statusScreen_(statusScreen)
                , save_(std::move(save))
                , saveWriter_(std::move(saveWriter))
                , economy_(save_.campaign)
                , textures_(content.economy, content.battle)
                , interfaceFont_(content.economy.assets.at(gameplay::AssetId{"font.interface"}))
                , titleFont_(content.economy.assets.at(gameplay::AssetId{"font.title"}))
                , abilityFont_(content.economy.assets.at(gameplay::AssetId{"font.ability"}))
                , priceFont_(content.economy.assets.at(gameplay::AssetId{"font.price"}))
            {
            }

            [[nodiscard]] bool run()
            {
                if (save_.activeLevel)
                {
                    statusScreen_.renderLoading();
                    const auto campaign = content_.world.levels.at(save_.activeLevel->run.level()).campaign;
                    if (playNextLevel(campaign)) return true;
                }
                MainRoomScreen room{window_, content_.flow, content_.world, save_};
                while (window_.isOpen())
                {
                    const auto result = room.run();
                    if (result.action == MainRoomAction::windowClosed)
                    {
                        return false;
                    }
                    if (result.action == MainRoomAction::mainMenu)
                    {
                        return true;
                    }
                    auto returnToMenu = false;
                    if (result.action == MainRoomAction::enterCampaign)
                    {
                        room.stopMusic();
                        statusScreen_.renderLoading();
                        returnToMenu = playNextLevel(result.campaign);
                    }
                    else
                    {
                        openPanel(result, room);
                    }
                    saveAtBoundary();
                    if (returnToMenu)
                    {
                        return true;
                    }
                    room.resume();
                }
                return false;
            }

            [[nodiscard]] const persistence::SaveGame& save() const noexcept
            {
                return save_;
            }

          private:
            bool playNextLevel(std::string_view campaign)
            {
                const auto level = save_.activeLevel ? std::optional{save_.activeLevel->run.level()}
                                                     : game::CampaignProgression{content_.world, save_.completedLevels}.nextLevel(campaign);
                if (!level)
                {
                    return false;
                }
                const auto baseline = game::CampaignLifecycleService::baselineForLevel(save_.campaign, save_.activeLevel);
                auto outcome = runCampaignExplorationSession(window_,
                                                             content_.battle,
                                                             content_.economy,
                                                             content_.world,
                                                             content_.flow,
                                                             content_.rules,
                                                             *level,
                                                             save_.campaign,
                                                             economy_,
                                                             std::random_device{}(),
                                                             std::move(save_.activeLevel),
                                                             [this]() {
                                                                 if (statusScreen_.processEvents()) statusScreen_.renderLoading();
                                                             });
                if (outcome.result == game::LevelSessionResult::completed || outcome.result == game::LevelSessionResult::defeated)
                {
                    sounds_.click();
                }
                const auto transition = game::CampaignLifecycleService::apply(std::move(outcome),
                                                                               *level,
                                                                               baseline,
                                                                               save_.campaign,
                                                                               economy_,
                                                                               save_.completedLevels,
                                                                               save_.activeLevel);
                if (!transition.hasValue())
                {
                    throw std::runtime_error(transition.error());
                }
                return transition.value().returnToMainMenu;
            }

            void openPanel(const MainRoomResult& result, MainRoomScreen& room)
            {
                const auto drawBackground = [&room]() { room.renderBackdrop(); };
                const EconomyPanelFonts fonts{interfaceFont_, titleFont_, abilityFont_, priceFont_};
                auto requestedAction = result.action;
                window_.setView(sf::View{sf::FloatRect{{0.0F, 0.0F}, {static_cast<float>(content_.economy.presentation.referenceSize.x),
                                                                     static_cast<float>(content_.economy.presentation.referenceSize.y)}}});
                while (window_.isOpen() && requestedAction != MainRoomAction::none)
                {
                    auto nextAction = MainRoomAction::none;
                    const auto navigate = [this, &room, &nextAction](sf::Vector2f position) {
                        const auto action = room.toolbarActionAt(position);
                        if (!action) return false;
                        sounds_.click();
                        nextAction = *action;
                        return true;
                    };
                    const auto navigationHovered = [&room](sf::Vector2f position) {
                        return room.toolbarActionAt(position).has_value();
                    };
                    const PanelHostOptions host{
                        .closeWindowOnExit = false,
                        .drawBackground = drawBackground,
                        .navigate = navigate,
                        .navigationHovered = navigationHovered,
                    };
                    if (requestedAction == MainRoomAction::inventory)
                    {
                        InventoryPanel panel{window_, content_.battle, content_.economy, save_.campaign, economy_, textures_, fonts, host};
                        if (result.player) panel.showPlayer(*result.player);
                        runPanel(panel);
                    }
                    else if (requestedAction == MainRoomAction::skills)
                    {
                        SkillPanel panel{window_, content_.battle, content_.economy, save_.campaign, economy_, textures_, fonts, host};
                        if (result.player) panel.showPlayer(*result.player);
                        runPanel(panel);
                    }
                    else if (requestedAction == MainRoomAction::shop)
                    {
                        game::RuleBuilderService ruleService{save_.campaign, content_.rules};
                        ShopPanel panel{window_, content_.economy, content_.rules, save_.campaign, economy_, ruleService, textures_, fonts, host};
                        runPanel(panel);
                    }
                    else if (requestedAction == MainRoomAction::rules)
                    {
                        RuleBuilderPanel panel{window_, content_.rules, save_.campaign, interfaceFont_, titleFont_, drawBackground, navigate,
                                               navigationHovered};
                        runPanel(panel);
                    }
                    requestedAction = nextAction;
                }
            }

            void saveAtBoundary()
            {
                const auto saved = saveWriter_(save_);
                if (!saved.hasValue())
                {
                    std::cerr << "Save failed: " << saved.error() << '\n';
                    statusScreen_.showError("Der Spielstand konnte nicht gespeichert werden: " + saved.error());
                }
            }

            template <typename Panel>
            void runPanel(Panel& panel)
            {
                while (window_.isOpen() && !panel.closeRequested())
                {
                    panel.processEvents();
                    panel.render();
                }
            }

            sf::RenderWindow& window_;
            const LoadedGameContent& content_;
            StartupScreen& statusScreen_;
            persistence::SaveGame save_;
            SaveWriter saveWriter_;
            game::EconomyService economy_;
            EconomySoundPlayer sounds_;
            EconomyTextures textures_;
            sf::Font interfaceFont_;
            sf::Font titleFont_;
            sf::Font abilityFont_;
            sf::Font priceFont_;
        };
    } // namespace

    int runGameApplication(const char* executablePath, std::optional<std::filesystem::path> saveGamePath)
    {
        const auto repositoryRoot = findSomaRepositoryRoot(executablePath);
        if (!repositoryRoot)
        {
            std::cerr << "Could not locate the Soma Data directory.\n";
            return 1;
        }
        std::filesystem::current_path(*repositoryRoot);
        auto flow = content::GameFlowContentLoader{}.load("Data/content/v1");
        if (!flow.valid())
        {
            std::cerr << formatContentDiagnostics("Game-flow content could not be loaded.", flow.diagnostics) << '\n';
            return 1;
        }

        auto window = makeWindow(flow.content);
        StartupScreen startup{window, flow.content};
        sf::Clock minimumDisplay;
        auto loading = std::async(std::launch::async, loadGameContent, flow.content);
        const auto minimumSeconds = static_cast<float>(flow.content.startup.minimumDisplaySeconds);
        while (window.isOpen() && (loading.wait_for(std::chrono::milliseconds{0}) != std::future_status::ready ||
                                   minimumDisplay.getElapsedTime().asSeconds() < minimumSeconds))
        {
            if (!startup.processEvents())
            {
                break;
            }
            startup.renderLoading();
        }
        auto loaded = loading.get();
        if (!loaded.content)
        {
            std::cerr << loaded.error << '\n';
            if (window.isOpen())
            {
                startup.showError(loaded.error);
            }
            return 1;
        }
        auto content = std::move(*loaded.content);
        if (!window.isOpen())
        {
            return 0;
        }

        persistence::SaveRepository repository{content.economy, content.world, content.rules};
        persistence::SaveSlotService slots{"Data/Saves", repository};
        if (saveGamePath)
        {
            const auto loadedSave = repository.load(*saveGamePath);
            if (!loadedSave.hasValue())
            {
                const auto error = "Save game could not be loaded: " + loadedSave.error();
                std::cerr << error << '\n';
                startup.showError(error);
                return 1;
            }
            const auto saveWriter = [&repository, path = *saveGamePath](const persistence::SaveGame& save) {
                return repository.save(path, save);
            };
            GameSession session{window, content, startup, loadedSave.value(), saveWriter};
            static_cast<void>(session.run());
            const auto saved = saveWriter(session.save());
            if (!saved.hasValue())
            {
                std::cerr << "Save failed: " << saved.error() << '\n';
                if (window.isOpen())
                {
                    startup.showError("Der Spielstand konnte nicht gespeichert werden: " + saved.error());
                }
                return 1;
            }
            return 0;
        }
        while (window.isOpen())
        {
            window.setView(sf::View{sf::FloatRect{{0.0F, 0.0F}, {static_cast<float>(content.flow.referenceSize.x),
                                                                 static_cast<float>(content.flow.referenceSize.y)}}});
            const auto selection = MainMenuScreen{window, content.flow, slots}.run();
            if (!selection)
            {
                break;
            }
            const auto saveWriter = [&slots, slot = selection->slot](const persistence::SaveGame& save) {
                return slots.store(slot, save);
            };
            GameSession session{window, content, startup, selection->saveGame, saveWriter};
            const auto returnToMenu = session.run();
            const auto saved = saveWriter(session.save());
            if (!saved.hasValue())
            {
                std::cerr << "Save failed: " << saved.error() << '\n';
                if (window.isOpen())
                {
                    startup.showError("Der Spielstand konnte nicht gespeichert werden: " + saved.error());
                }
            }
            if (!returnToMenu)
            {
                break;
            }
        }
        return 0;
    }
} // namespace soma::presentation
