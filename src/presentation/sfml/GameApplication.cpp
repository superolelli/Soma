#include "presentation/sfml/GameApplication.hpp"

#include "content/BattleContentLoader.hpp"
#include "content/EconomyContentLoader.hpp"
#include "content/GameFlowContentLoader.hpp"
#include "content/RuleContentLoader.hpp"
#include "content/WorldContentLoader.hpp"
#include "game/EconomyService.hpp"
#include "game/CampaignProgression.hpp"
#include "persistence/SaveRepository.hpp"
#include "persistence/SaveSlotService.hpp"
#include "presentation/sfml/BattleLaunchOptions.hpp"
#include "presentation/sfml/EconomySoundPlayer.hpp"
#include "presentation/sfml/EconomyTextures.hpp"
#include "presentation/sfml/InventoryPanel.hpp"
#include "presentation/sfml/ExplorationSession.hpp"
#include "presentation/sfml/MainMenuScreen.hpp"
#include "presentation/sfml/MainRoomScreen.hpp"
#include "presentation/sfml/ShopPanel.hpp"
#include "presentation/sfml/SkillPanel.hpp"
#include "presentation/sfml/RuleBuilderPanel.hpp"
#include "presentation/sfml/StartupScreen.hpp"

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <random>

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

        std::string diagnostics(std::string_view heading, const std::vector<gameplay::Diagnostic>& values)
        {
            std::string result{heading};
            for (const auto& diagnostic : values)
            {
                result += "\n" + diagnostic.file.string() + ": " + diagnostic.message;
            }
            return result;
        }

        GameContentLoadResult loadGameContent(content::GameFlowContent flow)
        {
            constexpr auto contentRoot = "Data/content/v1";
            auto battle = content::BattleContentLoader{}.load(contentRoot);
            auto economy = content::EconomyContentLoader{}.load(contentRoot);
            auto rules = content::RuleContentLoader{}.load(contentRoot);
            if (!battle.valid())
            {
                return {{}, diagnostics("Battle content could not be loaded.", battle.diagnostics)};
            }
            if (!economy.valid())
            {
                return {{}, diagnostics("Economy content could not be loaded.", economy.diagnostics)};
            }
            if (!rules.valid())
            {
                return {{}, diagnostics("Rule content could not be loaded.", rules.diagnostics)};
            }
            auto world = content::WorldContentLoader{}.load(contentRoot, battle.content, economy.content);
            if (!world.valid())
            {
                return {{}, diagnostics("World content could not be loaded.", world.diagnostics)};
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
                const auto stateBeforeLevel = save_.campaign;
                const auto baseline = save_.activeLevel ? save_.activeLevel->baseline : stateBeforeLevel.data();
                const auto outcome = runCampaignExplorationSession(window_,
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
                save_.activeLevel = outcome.savedLevel;
                if (outcome.result == ExplorationResult::completed || outcome.result == ExplorationResult::defeated)
                {
                    sounds_.click();
                }
                if (outcome.result == ExplorationResult::completed)
                {
                    save_.completedLevels.insert(*level);
                }
                if (outcome.result == ExplorationResult::completed || outcome.result == ExplorationResult::defeated)
                {
                    economy_.clearShopStock();
                }
                if (outcome.result == ExplorationResult::abandonAndMainMenu)
                {
                    save_.campaign = game::CampaignState::fromData(baseline);
                    save_.activeLevel.reset();
                }
                return outcome.result == ExplorationResult::saveAndMainMenu;
            }

            void openPanel(const MainRoomResult& result, MainRoomScreen& room)
            {
                const auto drawBackground = [&room]() { room.renderBackdrop(); };
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
                    if (requestedAction == MainRoomAction::inventory)
                    {
                        InventoryPanel panel{window_, content_.battle, content_.economy, save_.campaign, economy_, textures_, interfaceFont_,
                                             titleFont_, false, drawBackground, navigate, navigationHovered};
                        if (result.player) panel.showPlayer(*result.player);
                        runPanel(panel);
                    }
                    else if (requestedAction == MainRoomAction::skills)
                    {
                        SkillPanel panel{window_, content_.battle, content_.economy, save_.campaign, economy_, textures_, interfaceFont_,
                                         titleFont_, abilityFont_, priceFont_, false, drawBackground, navigate, navigationHovered};
                        if (result.player) panel.showPlayer(*result.player);
                        runPanel(panel);
                    }
                    else if (requestedAction == MainRoomAction::shop)
                    {
                        game::RuleBuilderService ruleService{save_.campaign, content_.rules};
                        ShopPanel panel{window_, content_.economy, content_.rules, save_.campaign, economy_, ruleService, textures_, interfaceFont_,
                                        titleFont_, priceFont_, false, drawBackground, navigate, navigationHovered};
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
            std::cerr << diagnostics("Game-flow content could not be loaded.", flow.diagnostics) << '\n';
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
