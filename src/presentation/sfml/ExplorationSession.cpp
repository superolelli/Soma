#include "presentation/sfml/ExplorationSession.hpp"

#include "game/CampaignBattleContentBuilder.hpp"
#include "game/ExplorationConsumableUseService.hpp"
#include "gameplay/random/SeededRandom.hpp"
#include "presentation/sfml/BattleActionPresentation.hpp"
#include "presentation/sfml/BattleScreenSession.hpp"
#include "presentation/sfml/BattleScreenView.hpp"
#include "presentation/sfml/EconomySoundPlayer.hpp"
#include "presentation/sfml/EconomyTextures.hpp"
#include "presentation/sfml/ExplorationConsumablePanel.hpp"
#include "presentation/sfml/ExplorationHud.hpp"
#include "presentation/sfml/ExplorationMusicPlayer.hpp"
#include "presentation/sfml/ExplorationPartyStatus.hpp"
#include "presentation/sfml/ExplorationSpeechBubbles.hpp"
#include "presentation/sfml/LevelResultDialog.hpp"
#include "presentation/sfml/LootDialog.hpp"
#include "presentation/sfml/PauseMenu.hpp"
#include "presentation/sfml/SfeMovieVideoPlayer.hpp"
#include "presentation/sfml/SpriterAnimationPlayer.hpp"
#include "simulation/exploration/LevelGenerator.hpp"
#include "simulation/rewards/LevelRewardCalculator.hpp"
#include "simulation/rewards/LootGenerator.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <ranges>
#include <span>
#include <unordered_map>
#include <unordered_set>

namespace soma::presentation
{
    namespace
    {
        const std::array<gameplay::ContentDefinitionId, 4>& playerDefinitions()
        {
            static const std::array players{gameplay::ContentDefinitionId{"player.simon"},
                                            gameplay::ContentDefinitionId{"player.ole"},
                                            gameplay::ContentDefinitionId{"player.anna"},
                                            gameplay::ContentDefinitionId{"player.markus"}};
            return players;
        }

        gameplay::ContentDefinitionId battleEncounterFor(const content::LevelDefinition& level)
        {
            return gameplay::ContentDefinitionId{level.campaign == "kutschfahrt" ? "encounter.kutschfahrt.default" : "encounter.bang.default"};
        }

        std::vector<std::filesystem::path> campaignMusicTracks(const content::WorldContent& world, const content::LevelDefinition& level)
        {
            std::vector<std::filesystem::path> result;
            for (const auto& asset : world.campaignPresentation.at(level.campaign).musicTracks)
            {
                result.push_back(world.assets.at(asset));
            }
            return result;
        }

        std::set<content::RuleFragmentType> discoveredRuleTypes(const game::CampaignState& state, const content::RuleContent& rules)
        {
            std::set<content::RuleFragmentType> result;
            for (const auto& [id, definition] : rules.fragments)
            {
                if (state.ruleFragmentCount(id) > 0)
                {
                    result.insert(definition.type);
                }
            }
            return result;
        }

        std::set<content::RuleFragmentType> discoveredRuleTypes(const game::CampaignStateData& state, const content::RuleContent& rules)
        {
            std::set<content::RuleFragmentType> result;
            for (const auto& [id, count] : state.ruleFragments)
            {
                const auto definition = rules.fragments.find(id);
                if (count > 0 && definition != rules.fragments.end())
                {
                    result.insert(definition->second.type);
                }
            }
            return result;
        }

        /// Coordinates one level from generation/loading through traversal, battles, loot, and final rewards.
        class ExplorationSession
        {
          public:
            ExplorationSession(const content::BattleContent& battle,
                               const content::EconomyContent& economy,
                               const content::WorldContent& world,
                               const content::RuleContent& rules,
                               gameplay::LevelId level,
                               game::CampaignState& campaignState,
                               game::EconomyService& economyService,
                               std::uint64_t randomSeed,
                               std::optional<simulation::LevelRun> authoredLevel,
                               sf::RenderWindow* sharedWindow = nullptr,
                               const content::GameFlowContent* flow = nullptr,
                               std::function<void()> loadingFrame = {},
                               std::optional<game::ActiveLevelState> savedLevel = std::nullopt)
                : battle_(battle)
                , economy_(economy)
                , world_(world)
                , rules_(rules)
                , levelId_(std::move(level))
                , campaignState_(campaignState)
                , economyService_(economyService)
                , ruleService_(campaignState, rules)
                , baselineCampaign_(savedLevel ? savedLevel->baseline : campaignState.data())
                , initiallyUnlockedItems_(baselineCampaign_.unlockedItems)
                , initiallyDiscoveredRuleTypes_(discoveredRuleTypes(baselineCampaign_, rules))
                , campaignBattle_(game::CampaignBattleContentBuilder{}.build(battle, economy, campaignState))
                , random_(randomSeed)
                , speechRandom_(randomSeed + 1U)
                , levelRun_(savedLevel      ? savedLevel->run
                            : authoredLevel ? std::move(*authoredLevel)
                                            : simulation::LevelGenerator{world, random_}.generate(levelId_))
                , music_(campaignMusicTracks(world, world.levels.at(levelId_)),
                         random_,
                         static_cast<float>(world.campaignPresentation.at(world.levels.at(levelId_).campaign).musicVolume),
                         static_cast<float>(world.campaignPresentation.at(world.levels.at(levelId_).campaign).battleMusicVolume),
                         static_cast<float>(world.campaignPresentation.at(world.levels.at(levelId_).campaign).battleCrossfadeSeconds))
                , layout_(world.presentation)
                , ownedWindow_(sharedWindow == nullptr
                                   ? std::make_unique<sf::RenderWindow>(sf::VideoMode::getDesktopMode(), "Soma - Level", sf::State::Fullscreen)
                                   : nullptr)
                , window_(sharedWindow != nullptr ? *sharedWindow : *ownedWindow_)
                , embedded_(sharedWindow != nullptr)
                , loadingFrame_(loadingFrame)
                , flow_(flow)
                , view_(sf::FloatRect{{0.0F, 0.0F}, {static_cast<float>(layout_.referenceSize.x), static_cast<float>(layout_.referenceSize.y)}})
                , hudView_(sf::FloatRect{{0.0F, 0.0F}, {static_cast<float>(layout_.referenceSize.x), static_cast<float>(layout_.referenceSize.y)}})
                , font_(economy.assets.at(gameplay::AssetId{"font.interface"}))
                , economyTextures_(economy, loadingFrame)
                , battleTextures_(battle)
                , lootGenerator_(world, economy, rules, random_)
                , explorationConsumableUse_(economyService)
            {
                if (flow != nullptr)
                {
                    pauseMenu_ = std::make_unique<PauseMenu>(*flow, PauseMenuContext::level);
                }
                showLoadingFrame();
                initializePlayerHealth();
                initializePlayerFatigue();
                if (savedLevel)
                {
                    playerHealth_ = savedLevel->playerHealth;
                    playerFatigue_ = savedLevel->playerFatigue;
                }
                loadWorldTextures();
                showLoadingFrame();
                speechBubbles_ = std::make_unique<ExplorationSpeechBubbles>(
                    layout_.speechBubbles, textures_.at(layout_.speechBubbles.texture), font_, speechRandom_);
                partyStatus_ = std::make_unique<ExplorationPartyStatus>(world_, font_);
                explorationHud_ = std::make_unique<ExplorationHud>(economy_, campaignState_);
                explorationConsumables_ = std::make_unique<ExplorationConsumablePanel>(economy_, campaignState_, economyTextures_, font_);
                loadCursors();
                lootableOutlineAvailable_ =
                    lootableOutlineShader_.loadFromFile(world_.assets.at(gameplay::AssetId{"lootable.outline_shader"}), sf::Shader::Type::Fragment);
                lootableOutlineShader_.setUniform("u_texture", sf::Shader::CurrentTexture);
                lootableOutlineShader_.setUniform("u_outlineColor", sf::Glsl::Vec4{1.0F, 0.72F, 0.18F, 1.0F});
                lootableOutlineShader_.setUniform("u_textureRect", sf::Glsl::Vec4{0.0F, 0.0F, 0.5F, 1.0F});
                roomWidth_ = static_cast<float>(textures_.at(levelRun_.rooms().front().background).getSize().x);
                maximumViewLeft_ = roomWidth_ * static_cast<float>(levelRun_.rooms().size()) - static_cast<float>(layout_.referenceSize.x);
                if (savedLevel)
                {
                    view_.setCenter({std::clamp(savedLevel->viewLeft, 0.0F, maximumViewLeft_) + view_.getSize().x / 2.0F, view_.getCenter().y});
                }
                createParty();
                showLoadingFrame();
                const auto& transition = layout_.battleTransition;
                battleTransition_ = std::make_unique<SpriterAnimationPlayer>(window_, world_.assets.at(transition.model), transition.entity);
                battleEffect_ = std::make_unique<SpriterAnimationPlayer>(window_, battle_.assets.at("ability_effect_model"), "ability_effects");
                rememberLevelBeforePendingEncounter();
                prepareUpcomingBattleViews();
                window_.setView(view_);
                window_.setVerticalSyncEnabled(true);
            }

            ExplorationResult run()
            {
                while (window_.isOpen() && !returnRequested_)
                {
                    const auto startBattle = processEvents() || pendingEncounter_.has_value();
                    if (pauseMenu_ && pauseMenu_->isOpen())
                    {
                        render();
                        continue;
                    }
                    if (startBattle && !defeated_ && !runBattle())
                    {
                        break;
                    }
                    update(clock_.restart());
                    if (!startBattle && pendingEncounter_.has_value() && !defeated_ && !runBattle())
                    {
                        break;
                    }
                    music_.update();
                    updateCursor();
                    render();
                }
                return result_;
            }

            [[nodiscard]] game::ActiveLevelState activeLevelState() const
            {
                return {levelRun_, playerHealth_, playerFatigue_, view_.getCenter().x - view_.getSize().x / 2.0F, baselineCampaign_};
            }

          private:
            void showLoadingFrame()
            {
                if (loadingFrame_)
                {
                    loadingFrame_();
                }
            }

            void rememberLevelBeforePendingEncounter()
            {
                levelRunBeforeBattle_ = levelRun_;
                pendingEncounter_ = levelRun_.takePendingEncounter();
                if (!pendingEncounter_)
                {
                    levelRunBeforeBattle_.reset();
                }
            }

            void requestSessionClose()
            {
                if (embedded_)
                {
                    returnRequested_ = true;
                }
                else
                {
                    window_.close();
                }
            }

            void initializePlayerHealth()
            {
                for (const auto& [id, definition] : campaignBattle_.combatants)
                {
                    if (definition.team == gameplay::Team::players)
                    {
                        playerHealth_.emplace(id, definition.attributes.maximumHealth);
                    }
                }
            }

            void initializePlayerFatigue()
            {
                for (const auto& id : playerDefinitions())
                {
                    playerFatigue_.emplace(id, 0);
                }
            }

            void loadWorldTextures()
            {
                for (const auto& [id, path] : world_.assets)
                {
                    if (id.value().ends_with(".sprite") || id.value().starts_with("background.") || id.value().starts_with("parallax.") ||
                        id == layout_.speechBubbles.texture)
                    {
                        textures_.emplace(id, sf::Texture{path});
                        showLoadingFrame();
                    }
                }
            }

            void loadCursors()
            {
                arrowCursorImage_.emplace(world_.assets.at(gameplay::AssetId{"cursor.normal"}));
                handCursorImage_.emplace(world_.assets.at(gameplay::AssetId{"cursor.hand"}));
                arrowCursor_.emplace(arrowCursorImage_->getPixelsPtr(), arrowCursorImage_->getSize(), sf::Vector2u{0U, 0U});
                handCursor_.emplace(handCursorImage_->getPixelsPtr(), handCursorImage_->getSize(), sf::Vector2u{0U, 0U});
                window_.setMouseCursor(*arrowCursor_);
            }

            void createParty()
            {
                for (const auto& player : playerDefinitions())
                {
                    const auto& presentation = battle_.presentation.at(player);
                    auto animation = std::make_unique<SpriterAnimationPlayer>(window_, presentation.model, presentation.entity);
                    animation->setScale(static_cast<float>(layout_.partyScale), static_cast<float>(layout_.partyScale));
                    animation->play(layout_.idleAnimation, static_cast<float>(layout_.idleAnimationSpeed));
                    party_.push_back(std::move(animation));
                }
            }

            bool processEvents()
            {
                bool startBattle{};
                while (const auto event = window_.pollEvent())
                {
                    if (event->is<sf::Event::Closed>())
                    {
                        window_.close();
                        continue;
                    }
                    if (pauseMenu_)
                    {
                        const auto action = pauseMenu_->handleEvent(*event, window_);
                        if (action == PauseMenuAction::saveAndMainMenu)
                        {
                            result_ = ExplorationResult::saveAndMainMenu;
                            requestSessionClose();
                            continue;
                        }
                        if (action == PauseMenuAction::abandonAndMainMenu)
                        {
                            result_ = ExplorationResult::abandonAndMainMenu;
                            requestSessionClose();
                            continue;
                        }
                        if (action || pauseMenu_->isOpen())
                        {
                            continue;
                        }
                    }
                    if (const auto* key = event->getIf<sf::Event::KeyPressed>())
                    {
                        startBattle = handleKey(*key) || startBattle;
                    }
                    if (const auto* click = event->getIf<sf::Event::MouseButtonReleased>(); click && click->button == sf::Mouse::Button::Left)
                    {
                        handleConsumableRelease(click->position);
                        handleClick(click->position);
                    }
                    if (const auto* press = event->getIf<sf::Event::MouseButtonPressed>();
                        press && press->button == sf::Mouse::Button::Left && !lootDialog_ && !levelResultDialog_ && !pendingEncounter_)
                    {
                        explorationConsumables_->press(window_.mapPixelToCoords(press->position, hudView_));
                    }
                }
                return startBattle;
            }

            void handleConsumableRelease(sf::Vector2i pixel)
            {
                const auto item = explorationConsumables_->release();
                if (!item)
                {
                    return;
                }
                const auto position = window_.mapPixelToCoords(pixel, view_);
                const auto bounds = partyBounds();
                for (std::size_t index = 0; index < bounds.size(); ++index)
                {
                    const sf::FloatRect target{{bounds.at(index).left, bounds.at(index).top}, {bounds.at(index).width, bounds.at(index).height}};
                    if (target.contains(position))
                    {
                        const auto& id = playerDefinitions().at(index);
                        const auto used = explorationConsumableUse_.use(
                            economy_.items.at(*item), playerHealth_.at(id), effectiveMaximumHealth(id), playerBuffs_[id]);
                        if (used.hasValue())
                        {
                            explorationConsumables_->playConsumedSound();
                        }
                        return;
                    }
                }
            }

            bool handleKey(const sf::Event::KeyPressed& key)
            {
                if (key.code == sf::Keyboard::Key::Escape || (key.code == sf::Keyboard::Key::Enter && levelResultDialog_))
                {
                    requestSessionClose();
                    return false;
                }
                return key.code == sf::Keyboard::Key::Enter && pendingEncounter_.has_value();
            }

            void handleClick(sf::Vector2i pixel)
            {
                if (levelResultDialog_)
                {
                    if (levelResultDialog_->clicked(window_.mapPixelToCoords(pixel, hudView_)))
                    {
                        requestSessionClose();
                    }
                    return;
                }
                if (lootDialog_)
                {
                    lootDialog_->handleClick(window_.mapPixelToCoords(pixel, hudView_));
                    if (lootDialog_->closed())
                    {
                        lootDialog_.reset();
                        if (completeLevelAfterLoot_)
                        {
                            completeLevelAfterLoot_ = false;
                            openLevelResult(world_.levels.at(levelId_), true);
                        }
                    }
                    return;
                }
                if (!pendingEncounter_)
                {
                    tryOpenLootable(window_.mapPixelToCoords(pixel));
                }
            }

            void tryOpenLootable(sf::Vector2f mouse)
            {
                const auto lastVisibleRoom = std::min(levelRun_.currentRoom() + 1, levelRun_.rooms().size() - 1);
                for (auto roomIndex = levelRun_.currentRoom(); roomIndex <= lastVisibleRoom; ++roomIndex)
                {
                    if (tryOpenLootable(roomIndex, mouse))
                    {
                        return;
                    }
                }
            }

            void updateCursor()
            {
                const auto worldMouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), view_);
                const auto hudMouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_);
                const auto hand = isHoveringLootable(worldMouse) || (lootDialog_ && lootDialog_->clickableAt(hudMouse)) ||
                                  (levelResultDialog_ && levelResultDialog_->clicked(hudMouse)) ||
                                  (!lootDialog_ && !levelResultDialog_ && !pendingEncounter_ && explorationConsumables_->clickableAt(hudMouse));
                window_.setMouseCursor(hand ? *handCursor_ : *arrowCursor_);
            }

            bool isHoveringLootable(sf::Vector2f mouse) const
            {
                if (pendingEncounter_ || lootDialog_ || levelResultDialog_ || defeated_)
                {
                    return false;
                }
                const auto lastVisibleRoom = std::min(levelRun_.currentRoom() + 1, levelRun_.rooms().size() - 1);
                for (auto roomIndex = levelRun_.currentRoom(); roomIndex <= lastVisibleRoom; ++roomIndex)
                {
                    const auto& room = levelRun_.rooms().at(roomIndex);
                    if (!room.lootable || room.lootableOpened)
                    {
                        continue;
                    }
                    if (lootableHitbox(roomIndex).contains(mouse))
                    {
                        return true;
                    }
                }
                return false;
            }

            sf::FloatRect lootableHitbox(std::size_t roomIndex) const
            {
                const auto& room = levelRun_.rooms().at(roomIndex);
                const auto& definition = world_.lootables.at(*room.lootable);
                const auto roomLeft = static_cast<float>(roomIndex) * roomWidth_;
                return {{roomLeft + definition.position.x + definition.hitbox.x, static_cast<float>(definition.position.y + definition.hitbox.y)},
                        {static_cast<float>(definition.hitbox.width), static_cast<float>(definition.hitbox.height)}};
            }

            bool tryOpenLootable(std::size_t roomIndex, sf::Vector2f mouse)
            {
                const auto& room = levelRun_.rooms().at(roomIndex);
                if (!room.lootable || room.lootableOpened)
                {
                    return false;
                }
                const auto& definition = world_.lootables.at(*room.lootable);
                const auto roomLeft = static_cast<float>(roomIndex) * roomWidth_;
                const sf::FloatRect bounds{
                    {roomLeft + definition.position.x + definition.hitbox.x, static_cast<float>(definition.position.y + definition.hitbox.y)},
                    {static_cast<float>(definition.hitbox.width), static_cast<float>(definition.hitbox.height)}};
                if (!bounds.contains(mouse))
                {
                    return false;
                }
                const auto& level = world_.levels.at(levelId_);
                const simulation::RuleLootContext ruleContext{simulation::RuleRewardSource::lootable, level.campaign, level.id, {}};
                auto loot = lootGenerator_.generate(definition.lootTable, level.campaign, level.difficulty, ruleContext);
                if (levelRun_.openLootable(roomIndex))
                {
                    lootDialog_ = std::make_unique<LootDialog>(world_,
                                                               economy_,
                                                               rules_,
                                                               economyTextures_,
                                                               sounds_,
                                                               economyService_,
                                                               ruleService_,
                                                               definition.name,
                                                               world_.assets.at(definition.sound),
                                                               std::move(loot));
                    return true;
                }
                return false;
            }

            bool runBattle()
            {
                const auto encounterId = *pendingEncounter_;
                const auto campaignBeforeBattle = campaignState_;
                const auto& worldEncounter = world_.encounters.at(encounterId);
                const auto& level = world_.levels.at(levelId_);
                const auto& battleEncounter = campaignBattle_.encounters.at(battleEncounterFor(level));
                BattleScreenRequest request;
                request.encounter = battleEncounter;
                request.enemies = worldEncounter.combatants;
                request.background = world_.assets.at(levelRun_.room().background);
                appendBattleParallaxLayers(request);
                request.music.clear();
                request.onBattleTransition = [this, music = battleEncounter.music]() {
                    music_.pauseForBattle(music);
                };
                request.onBattleReady = [this]() {
                    music_.startBattleMusic();
                };
                request.randomSeed = static_cast<std::uint64_t>(random_.between(0, std::numeric_limits<int>::max()));
                request.playerHealth = playerHealth_;
                request.playerFatigue = playerFatigue_;
                request.playerBuffs = playerBuffs_;
                const auto& transition = layout_.battleTransition;
                request.transition = BattleTransitionRequest{world_.assets.at(transition.model),
                                                             transition.entity,
                                                             transition.animation,
                                                             {static_cast<float>(transition.position.x), static_cast<float>(transition.position.y)},
                                                             static_cast<float>(transition.scale),
                                                             static_cast<float>(transition.delayAfterSeconds)};
                request.preparedTransitionAnimation = battleTransition_.get();
                request.preparedEffectAnimation = battleEffect_.get();
                request.preparedTextures = &battleTextures_;
                if (preparedEncounter_ == pendingEncounter_)
                {
                    request.preparedCombatantViews = &preparedBattleViews_;
                }
                if (worldEncounter.introCue)
                {
                    request.introVideo = world_.assets.at(*worldEncounter.introCue);
                }
                request.returnWhenResolved = true;
                request.flow = flow_;
                const auto outcome =
                    runBattleScreenSession(window_, campaignBattle_, economy_, rules_, campaignState_, request, &videoPlayer_);
                if (outcome.result == BattleScreenResult::saveAndMainRoom || outcome.result == BattleScreenResult::abandonAndMainRoom)
                {
                    campaignState_ = campaignBeforeBattle;
                    if (levelRunBeforeBattle_)
                    {
                        levelRun_ = *levelRunBeforeBattle_;
                    }
                    result_ = outcome.result == BattleScreenResult::saveAndMainRoom ? ExplorationResult::saveAndMainMenu
                                                                                    : ExplorationResult::abandonAndMainMenu;
                    requestSessionClose();
                    return false;
                }
                if (outcome.result == BattleScreenResult::windowClosed)
                {
                    return false;
                }
                music_.resumeAfterBattle();
                applyBattleOutcome(worldEncounter, level, outcome);
                pendingEncounter_.reset();
                levelRunBeforeBattle_.reset();
                preparedEncounter_.reset();
                preparedBattleViews_.clear();
                prepareUpcomingBattleViews();
                window_.setView(view_);
                clock_.restart();
                return true;
            }

            void prepareUpcomingBattleViews()
            {
                std::optional<gameplay::EncounterId> encounter = pendingEncounter_;
                if (!encounter)
                {
                    for (std::size_t index = levelRun_.currentRoom() + 1; index < levelRun_.rooms().size(); ++index)
                    {
                        const auto& room = levelRun_.rooms().at(index);
                        if (room.encounterPending && room.encounter)
                        {
                            encounter = room.encounter;
                            break;
                        }
                    }
                }
                if (!encounter || preparedEncounter_ == encounter)
                {
                    return;
                }

                const auto& level = world_.levels.at(levelId_);
                const auto& battleEncounter = campaignBattle_.encounters.at(battleEncounterFor(level));
                const auto& enemies = world_.encounters.at(*encounter).combatants;
                gameplay::SeededRandom preparationRandom{0U};
                simulation::BattleSimulation preparationBattle{
                    campaignBattle_, createBattleSpawns(battleEncounter, enemies, playerHealth_, playerFatigue_, playerBuffs_), preparationRandom};
                preparedBattleViews_ = createCombatantViews(window_, campaignBattle_, preparationBattle);
                preparedEncounter_ = encounter;
            }

            void appendBattleParallaxLayers(BattleScreenRequest& request) const
            {
                const auto& parallax = levelRun_.parallax();
                const std::array fourthLayer{parallax.fourthLayer};
                appendBattleParallaxLayer(request, fourthLayer, parallax.fourthLayerDivisor);
                appendBattleParallaxLayer(request, parallax.thirdLayerTiles, parallax.thirdLayerDivisor);
                appendBattleParallaxLayer(request, parallax.secondLayerTiles, parallax.secondLayerDivisor);
            }

            void appendBattleParallaxLayer(BattleScreenRequest& request, std::span<const gameplay::AssetId> tiles, int scrollDivisor) const
            {
                if (tiles.empty() || scrollDivisor <= 0)
                {
                    return;
                }
                const auto viewLeft = view_.getCenter().x - view_.getSize().x / 2.0F;
                const auto parallaxDistance = viewLeft / static_cast<float>(scrollDivisor);
                const auto firstTile = static_cast<std::size_t>(parallaxDistance / roomWidth_);
                const auto firstPosition = -std::fmod(parallaxDistance, roomWidth_);
                for (std::size_t offset = 0; offset < 2; ++offset)
                {
                    const auto& asset = tiles[(firstTile + offset) % tiles.size()];
                    request.backgroundLayers.push_back({world_.assets.at(asset), {firstPosition + static_cast<float>(offset) * roomWidth_, 0.0F}});
                }
            }

            void applyBattleOutcome(const content::EncounterDefinition& encounter,
                                    const content::LevelDefinition& level,
                                    const BattleScreenOutcome& outcome)
            {
                playerHealth_ = outcome.playerHealth;
                playerFatigue_ = outcome.playerFatigue;
                playerBuffs_ = outcome.playerBuffs;
                defeated_ = outcome.result == BattleScreenResult::enemiesWon;
                usedPlayerAbilities_.insert(outcome.usedPlayerAbilities.begin(), outcome.usedPlayerAbilities.end());
                if (outcome.result == BattleScreenResult::playersWon)
                {
                    enemiesDefeated_ += static_cast<int>(encounter.combatants.size());
                }
                if (outcome.result == BattleScreenResult::playersWon && !encounter.boss)
                {
                    openBattleRewards(encounter, level);
                }
                if (outcome.result == BattleScreenResult::playersWon && encounter.boss)
                {
                    const simulation::RuleLootContext ruleContext{
                        simulation::RuleRewardSource::battle, level.campaign, level.id, encounter.combatants};
                    if (const auto fragment = lootGenerator_.rollRuleFragment(ruleContext))
                    {
                        simulation::LootBundle loot;
                        loot.ruleFragment = fragment;
                        lootDialog_ = std::make_unique<LootDialog>(world_,
                                                                   economy_,
                                                                   rules_,
                                                                   economyTextures_,
                                                                   sounds_,
                                                                   economyService_,
                                                                   ruleService_,
                                                                   "Kampf gewonnen!",
                                                                   std::nullopt,
                                                                   std::move(loot));
                        completeLevelAfterLoot_ = true;
                    }
                    else
                    {
                        openLevelResult(level, true);
                    }
                }
                if (defeated_)
                {
                    openLevelResult(level, false);
                }
            }

            void openBattleRewards(const content::EncounterDefinition& encounter, const content::LevelDefinition& level)
            {
                const simulation::RuleLootContext ruleContext{simulation::RuleRewardSource::battle, level.campaign, level.id, encounter.combatants};
                auto rewards = lootGenerator_.generate(encounter.rewardTable, level.campaign, level.difficulty, ruleContext);
                lootDialog_ = std::make_unique<LootDialog>(world_,
                                                           economy_,
                                                           rules_,
                                                           economyTextures_,
                                                           sounds_,
                                                           economyService_,
                                                           ruleService_,
                                                           "Kampf gewonnen!",
                                                           std::nullopt,
                                                           std::move(rewards));
            }

            void openLevelResult(const content::LevelDefinition& level, bool completed)
            {
                result_ = completed ? ExplorationResult::completed : ExplorationResult::defeated;
                const auto playerDeaths = static_cast<int>(std::ranges::count_if(playerHealth_, [](const auto& entry) {
                    return entry.second <= 0;
                }));
                const auto highestFatigue = std::ranges::max(playerFatigue_ | std::views::values);
                auto rewards = simulation::LevelRewardCalculator{}.calculate(
                    {level.difficulty, completed, playerDeaths, enemiesDefeated_, usedPlayerAbilities_.size() == 16, highestFatigue});
                auto unlockedItems = newlyUnlockedItems();
                auto unlockedRuleTypes = newlyDiscoveredRuleTypes();
                grantLevelRewards(rewards);
                levelResultDialog_ = std::make_unique<LevelResultDialog>(world_,
                                                                         economy_,
                                                                         rules_,
                                                                         economyTextures_,
                                                                         level.campaign,
                                                                         completed,
                                                                         std::move(rewards),
                                                                         std::move(unlockedItems),
                                                                         std::move(unlockedRuleTypes));
            }

            std::vector<gameplay::ItemId> newlyUnlockedItems() const
            {
                std::vector<gameplay::ItemId> result;
                for (const auto& item : campaignState_.unlockedItems())
                {
                    if (!initiallyUnlockedItems_.contains(item) && economy_.items.at(item).category != gameplay::ItemCategory::currency)
                    {
                        result.push_back(item);
                    }
                }
                return result;
            }

            std::vector<content::RuleFragmentType> newlyDiscoveredRuleTypes() const
            {
                std::vector<content::RuleFragmentType> result;
                for (const auto type : discoveredRuleTypes(campaignState_, rules_))
                {
                    if (!initiallyDiscoveredRuleTypes_.contains(type))
                    {
                        result.push_back(type);
                    }
                }
                return result;
            }

            void grantLevelRewards(const simulation::LevelRewards& rewards)
            {
                if (rewards.totalCards() > 0)
                {
                    static_cast<void>(economyService_.grant(economy_.items.at(gameplay::ItemId{"currency.cards"}), rewards.totalCards()));
                }
                if (rewards.totalDice() > 0)
                {
                    static_cast<void>(economyService_.grant(economy_.items.at(gameplay::ItemId{"currency.dice"}), rewards.totalDice()));
                }
            }

            void update(sf::Time elapsed)
            {
                const auto direction = movementDirection();
                const auto viewLeft = std::clamp(view_.getCenter().x - view_.getSize().x / 2.0F +
                                                     direction * static_cast<float>(layout_.movementSpeed) * elapsed.asSeconds(),
                                                 0.0F,
                                                 maximumViewLeft_);
                view_.setCenter({viewLeft + view_.getSize().x / 2.0F, view_.getCenter().y});
                window_.setView(view_);
                enterVisibleRoom(viewLeft);
                updateParty(elapsed, direction, viewLeft);
                updateSpeechBubbles(elapsed);
                updatePartyStatus(elapsed);
            }

            void updatePartyStatus(sf::Time elapsed)
            {
                std::array<int, 4> health;
                std::array<int, 4> fatigue;
                std::array<bool, 4> buffed;
                for (std::size_t index = 0; index < health.size(); ++index)
                {
                    const auto& id = playerDefinitions().at(index);
                    health.at(index) = playerHealth_.at(id);
                    fatigue.at(index) = playerFatigue_.at(id);
                    buffed.at(index) = playerBuffs_.contains(id) && !playerBuffs_.at(id).empty();
                }
                partyStatus_->update(elapsed, health, fatigue, buffed);
            }

            void updateSpeechBubbles(sf::Time elapsed)
            {
                const auto bounds = partyBounds();
                const auto available = livingPartyMembers();
                const auto active = !pendingEncounter_ && !lootDialog_ && !levelResultDialog_ && !defeated_;
                speechBubbles_->update(elapsed, bounds, available, active);
            }

            std::array<AnimationBounds, 4> partyBounds()
            {
                std::array<AnimationBounds, 4> result;
                for (std::size_t index = 0; index < party_.size(); ++index)
                {
                    result.at(index) = party_.at(index)->bounds();
                }
                return result;
            }

            std::array<bool, 4> livingPartyMembers() const
            {
                std::array<bool, 4> result;
                for (std::size_t index = 0; index < result.size(); ++index)
                {
                    result.at(index) = playerHealth_.at(playerDefinitions().at(index)) > 0;
                }
                return result;
            }

            int effectiveMaximumHealth(const gameplay::ContentDefinitionId& id) const
            {
                auto maximum = campaignBattle_.combatants.at(id).attributes.maximumHealth;
                const auto buffs = playerBuffs_.find(id);
                if (buffs != playerBuffs_.end())
                {
                    for (const auto& buff : buffs->second)
                    {
                        maximum += buff.attributes.maximumHealth;
                    }
                }
                return maximum;
            }

            float movementDirection() const
            {
                if (pendingEncounter_ || lootDialog_ || levelResultDialog_ || defeated_)
                {
                    return 0.0F;
                }
                return (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ? 1.0F : 0.0F) -
                       (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ? 1.0F : 0.0F);
            }

            void enterVisibleRoom(float viewLeft)
            {
                const auto roomIndex = std::min(levelRun_.rooms().size() - 1, static_cast<std::size_t>(viewLeft / roomWidth_));
                if (roomIndex != levelRun_.currentRoom() && levelRun_.enterRoom(roomIndex))
                {
                    rememberLevelBeforePendingEncounter();
                }
            }

            void updateParty(sf::Time elapsed, float direction, float viewLeft)
            {
                const auto nowWalking = direction != 0.0F;
                if (nowWalking != walking_)
                {
                    walking_ = nowWalking;
                    for (auto& animation : party_)
                    {
                        animation->play(walking_ ? layout_.walkAnimation : layout_.idleAnimation,
                                        static_cast<float>(walking_ ? layout_.walkAnimationSpeed : layout_.idleAnimationSpeed));
                    }
                }
                for (std::size_t index = 0; index < party_.size(); ++index)
                {
                    if (playerHealth_.at(playerDefinitions().at(index)) <= 0)
                    {
                        continue;
                    }
                    party_.at(index)->setPosition(viewLeft + layout_.partyPositions.at(index).x,
                                                  static_cast<float>(layout_.partyPositions.at(index).y));
                    party_.at(index)->update(static_cast<float>(elapsed.asMilliseconds()));
                }
            }

            void render()
            {
                drawExplorationFrame();
                window_.display();
                window_.setView(view_);
            }

            void drawExplorationFrame()
            {
                window_.setView(view_);
                window_.clear(sf::Color::Black);
                renderParallax();
                renderRooms();
                renderParty();
                renderPartyStatus();
                speechBubbles_->render(window_);
                window_.setView(hudView_);
                renderHud();
            }

            void renderParallax()
            {
                const auto& parallax = levelRun_.parallax();
                const std::array fourthLayer{parallax.fourthLayer};
                renderParallaxLayer(fourthLayer, parallax.fourthLayerDivisor);
                renderParallaxLayer(parallax.thirdLayerTiles, parallax.thirdLayerDivisor);
                renderParallaxLayer(parallax.secondLayerTiles, parallax.secondLayerDivisor);
            }

            void renderParallaxLayer(std::span<const gameplay::AssetId> tiles, int scrollDivisor)
            {
                if (tiles.empty() || scrollDivisor <= 0)
                {
                    return;
                }
                const auto viewLeft = view_.getCenter().x - view_.getSize().x / 2.0F;
                const auto parallaxDistance = viewLeft / static_cast<float>(scrollDivisor);
                const auto firstTile = static_cast<std::size_t>(parallaxDistance / roomWidth_);
                const auto firstPosition = viewLeft - std::fmod(parallaxDistance, roomWidth_);
                for (std::size_t offset = 0; offset < 2; ++offset)
                {
                    const auto& texture = textures_.at(tiles[(firstTile + offset) % tiles.size()]);
                    sf::Sprite sprite{texture};
                    sprite.setPosition({firstPosition + static_cast<float>(offset) * roomWidth_, 0.0F});
                    window_.draw(sprite);
                }
            }

            void renderRooms()
            {
                const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), view_);
                for (std::size_t index = 0; index < levelRun_.rooms().size(); ++index)
                {
                    const auto& room = levelRun_.rooms().at(index);
                    sf::Sprite background{textures_.at(room.background)};
                    background.setPosition({static_cast<float>(index) * roomWidth_, 0.0F});
                    window_.draw(background);
                    if (room.lootable)
                    {
                        const auto& definition = world_.lootables.at(*room.lootable);
                        const auto& texture = textures_.at(definition.sprite);
                        const auto frameWidth = static_cast<int>(texture.getSize().x / 2U);
                        sf::Sprite lootable{texture};
                        lootable.setTextureRect({{room.lootableOpened ? frameWidth : 0, 0}, {frameWidth, static_cast<int>(texture.getSize().y)}});
                        lootable.setPosition(
                            {static_cast<float>(index) * roomWidth_ + definition.position.x, static_cast<float>(definition.position.y)});
                        if (!room.lootableOpened && lootableHitbox(index).contains(mouse) && lootableOutlineAvailable_)
                        {
                            lootableOutlineShader_.setUniform(
                                "u_textureSize", sf::Glsl::Vec2{static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y)});
                            constexpr int samples = 20;
                            constexpr float radius = 2.5F;
                            constexpr float tau = 6.28318530718F;
                            for (auto sample = 0; sample < samples; ++sample)
                            {
                                const auto angle = tau * static_cast<float>(sample) / static_cast<float>(samples);
                                auto outline = lootable;
                                outline.move({std::cos(angle) * radius, std::sin(angle) * radius});
                                window_.draw(outline, &lootableOutlineShader_);
                            }
                            window_.draw(lootable);
                        }
                        else
                        {
                            window_.draw(lootable);
                        }
                    }
                }
            }

            void renderParty()
            {
                for (std::size_t index = 0; index < party_.size(); ++index)
                {
                    if (playerHealth_.at(playerDefinitions().at(index)) > 0)
                    {
                        party_.at(index)->render();
                    }
                }
            }

            void renderPartyStatus()
            {
                std::array<int, 4> health;
                std::array<int, 4> maximumHealth;
                for (std::size_t index = 0; index < health.size(); ++index)
                {
                    const auto& id = playerDefinitions().at(index);
                    health.at(index) = playerHealth_.at(id);
                    maximumHealth.at(index) = effectiveMaximumHealth(id);
                }
                partyStatus_->render(window_, partyBounds(), health, maximumHealth, window_.mapPixelToCoords(sf::Mouse::getPosition(window_), view_));
            }

            void renderHud()
            {
                explorationHud_->render(
                    window_, campaignState_.currency(gameplay::Currency::cards), campaignState_.currency(gameplay::Currency::dice));
                const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_);
                if (!lootDialog_ && !levelResultDialog_ && !pendingEncounter_)
                {
                    explorationConsumables_->render(window_, mouse);
                }
                if (lootDialog_)
                {
                    lootDialog_->render(window_, mouse);
                }
                if (levelResultDialog_)
                {
                    levelResultDialog_->render(window_, mouse);
                }
                if (pauseMenu_)
                {
                    pauseMenu_->render(window_, mouse);
                }
            }

            const content::BattleContent& battle_;
            const content::EconomyContent& economy_;
            const content::WorldContent& world_;
            const content::RuleContent& rules_;
            gameplay::LevelId levelId_;
            game::CampaignState& campaignState_;
            game::EconomyService& economyService_;
            game::RuleBuilderService ruleService_;
            game::CampaignStateData baselineCampaign_;
            std::set<gameplay::ItemId> initiallyUnlockedItems_;
            std::set<content::RuleFragmentType> initiallyDiscoveredRuleTypes_;
            content::BattleContent campaignBattle_;
            gameplay::SeededRandom random_;
            gameplay::SeededRandom speechRandom_;
            simulation::LevelRun levelRun_;
            ExplorationMusicPlayer music_;
            SfeMovieVideoPlayer videoPlayer_;
            const content::WorldContent::Presentation& layout_;
            std::unique_ptr<sf::RenderWindow> ownedWindow_;
            sf::RenderWindow& window_;
            bool embedded_{};
            std::function<void()> loadingFrame_;
            const content::GameFlowContent* flow_{};
            bool returnRequested_{};
            std::unique_ptr<PauseMenu> pauseMenu_;
            sf::View view_;
            sf::View hudView_;
            sf::Font font_;
            EconomySoundPlayer sounds_;
            EconomyTextures economyTextures_;
            BattleTextures battleTextures_;
            std::unordered_map<gameplay::AssetId, sf::Texture> textures_;
            float roomWidth_{};
            float maximumViewLeft_{};
            std::vector<std::unique_ptr<SpriterAnimationPlayer>> party_;
            std::unique_ptr<SpriterAnimationPlayer> battleTransition_;
            std::unique_ptr<SpriterAnimationPlayer> battleEffect_;
            CombatantViews preparedBattleViews_;
            std::optional<gameplay::EncounterId> preparedEncounter_;
            std::unique_ptr<ExplorationSpeechBubbles> speechBubbles_;
            std::unique_ptr<ExplorationPartyStatus> partyStatus_;
            std::unique_ptr<ExplorationHud> explorationHud_;
            std::unique_ptr<ExplorationConsumablePanel> explorationConsumables_;
            std::optional<sf::Image> arrowCursorImage_;
            std::optional<sf::Image> handCursorImage_;
            std::optional<sf::Cursor> arrowCursor_;
            std::optional<sf::Cursor> handCursor_;
            sf::Shader lootableOutlineShader_;
            bool lootableOutlineAvailable_{};
            sf::Clock clock_;
            bool walking_{};
            std::optional<gameplay::EncounterId> pendingEncounter_;
            std::optional<simulation::LevelRun> levelRunBeforeBattle_;
            std::unique_ptr<LootDialog> lootDialog_;
            std::unique_ptr<LevelResultDialog> levelResultDialog_;
            bool completeLevelAfterLoot_{};
            simulation::LootGenerator lootGenerator_;
            game::ExplorationConsumableUseService explorationConsumableUse_;
            bool defeated_{};
            ExplorationResult result_{ExplorationResult::abandoned};
            int enemiesDefeated_{};
            std::unordered_set<gameplay::AbilityId> usedPlayerAbilities_;
            std::unordered_map<gameplay::ContentDefinitionId, int> playerHealth_;
            std::unordered_map<gameplay::ContentDefinitionId, int> playerFatigue_;
            std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> playerBuffs_;
        };
    } // namespace

    int runExplorationSession(const content::BattleContent& battle,
                              const content::EconomyContent& economy,
                              const content::WorldContent& world,
                              const content::RuleContent& rules,
                              const gameplay::LevelId& level,
                              game::CampaignState& campaignState,
                              game::EconomyService& economyService,
                              std::uint64_t randomSeed,
                              std::optional<simulation::LevelRun> authoredLevel)
    {
        static_cast<void>(
            ExplorationSession{battle, economy, world, rules, level, campaignState, economyService, randomSeed, std::move(authoredLevel)}.run());
        return 0;
    }

    ExplorationOutcome runCampaignExplorationSession(sf::RenderWindow& window,
                                                     const content::BattleContent& battle,
                                                     const content::EconomyContent& economy,
                                                     const content::WorldContent& world,
                                                     const content::GameFlowContent& flow,
                                                     const content::RuleContent& rules,
                                                     const gameplay::LevelId& level,
                                                     game::CampaignState& campaignState,
                                                     game::EconomyService& economyService,
                                                     std::uint64_t randomSeed,
                                                     std::optional<game::ActiveLevelState> savedLevel,
                                                     std::function<void()> loadingFrame)
    {
        ExplorationSession session{battle,
                                   economy,
                                   world,
                                   rules,
                                   level,
                                   campaignState,
                                   economyService,
                                   randomSeed,
                                   std::nullopt,
                                   &window,
                                   &flow,
                                   std::move(loadingFrame),
                                   std::move(savedLevel)};
        const auto result = session.run();
        return {result,
                result == ExplorationResult::saveAndMainMenu ? std::optional<game::ActiveLevelState>{session.activeLevelState()} : std::nullopt};
    }
} // namespace soma::presentation
