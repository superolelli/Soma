#include "game/CampaignBattleContentBuilder.hpp"
#include "presentation/sfml/ExplorationSessionController.hpp"
#include "simulation/exploration/LevelGenerator.hpp"

#include <algorithm>
#include <ranges>

namespace soma::presentation::detail
{
    const std::array<gameplay::ContentDefinitionId, 4>& ExplorationSession::playerDefinitions()
    {
        static const std::array players{gameplay::ContentDefinitionId{"player.simon"},
                                        gameplay::ContentDefinitionId{"player.ole"},
                                        gameplay::ContentDefinitionId{"player.anna"},
                                        gameplay::ContentDefinitionId{"player.markus"}};
        return players;
    }

    gameplay::ContentDefinitionId ExplorationSession::battleEncounterFor(const content::LevelDefinition& level)
    {
        return gameplay::ContentDefinitionId{level.campaign == "kutschfahrt" ? "encounter.kutschfahrt.default" : "encounter.bang.default"};
    }

    std::vector<std::filesystem::path> ExplorationSession::campaignMusicTracks(const content::WorldContent& world,
                                                                               const content::LevelDefinition& level)
    {
        std::vector<std::filesystem::path> result;
        for (const auto& asset : world.campaignPresentation.at(level.campaign).musicTracks)
        {
            result.push_back(world.assets.at(asset));
        }
        return result;
    }

    std::set<content::RuleFragmentType> ExplorationSession::discoveredRuleTypes(const game::CampaignState& state, const content::RuleContent& rules)
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

    std::set<content::RuleFragmentType> ExplorationSession::discoveredRuleTypes(const game::CampaignStateData& state,
                                                                                const content::RuleContent& rules)
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

    ExplorationSession::ExplorationSession(const content::BattleContent& battle,
                                           const content::EconomyContent& economy,
                                           const content::WorldContent& world,
                                           const content::RuleContent& rules,
                                           gameplay::LevelId level,
                                           game::CampaignState& campaignState,
                                           game::EconomyService& economyService,
                                           std::uint64_t randomSeed,
                                           std::optional<simulation::LevelRun> authoredLevel,
                                           sf::RenderWindow* sharedWindow,
                                           const content::GameFlowContent* flow,
                                           std::function<void()> loadingFrame,
                                           std::optional<game::ActiveLevelState> savedLevel)
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
        , particles_(resolveParticleEmitters(world.campaignPresentation.at(world.levels.at(levelId_).campaign).particles, world.assets),
                     randomSeed + 2U)
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
        speechBubbles_ =
            std::make_unique<ExplorationSpeechBubbles>(layout_.speechBubbles, textures_.at(layout_.speechBubbles.texture), font_, speechRandom_);
        partyStatus_ = std::make_unique<ExplorationPartyStatus>(world_, font_);
        explorationHud_ = std::make_unique<ExplorationHud>(economy_, campaignState_);
        explorationConsumables_ = std::make_unique<ExplorationConsumablePanel>(economy_, campaignState_, economyTextures_, font_);
        activeRules_ = std::make_unique<ActiveRulesPanel>(rules_, campaignState_.rules(), font_);
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

    ExplorationResult ExplorationSession::run()
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

    [[nodiscard]] game::ActiveLevelState ExplorationSession::activeLevelState() const
    {
        return {levelRun_, playerHealth_, playerFatigue_, view_.getCenter().x - view_.getSize().x / 2.0F, baselineCampaign_};
    }

    void ExplorationSession::showLoadingFrame()
    {
        if (loadingFrame_)
        {
            loadingFrame_();
        }
    }

    void ExplorationSession::rememberLevelBeforePendingEncounter()
    {
        levelRunBeforeBattle_ = levelRun_;
        pendingEncounter_ = levelRun_.takePendingEncounter();
        if (!pendingEncounter_)
        {
            levelRunBeforeBattle_.reset();
        }
    }

    void ExplorationSession::requestSessionClose()
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

    void ExplorationSession::initializePlayerHealth()
    {
        for (const auto& [id, definition] : campaignBattle_.combatants)
        {
            if (definition.team == gameplay::Team::players)
            {
                playerHealth_.emplace(id, definition.attributes.maximumHealth);
            }
        }
    }

    void ExplorationSession::initializePlayerFatigue()
    {
        for (const auto& id : playerDefinitions())
        {
            playerFatigue_.emplace(id, 0);
        }
    }

    void ExplorationSession::loadWorldTextures()
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

    void ExplorationSession::loadCursors()
    {
        arrowCursorImage_.emplace(world_.assets.at(gameplay::AssetId{"cursor.normal"}));
        handCursorImage_.emplace(world_.assets.at(gameplay::AssetId{"cursor.hand"}));
        arrowCursor_.emplace(arrowCursorImage_->getPixelsPtr(), arrowCursorImage_->getSize(), sf::Vector2u{0U, 0U});
        handCursor_.emplace(handCursorImage_->getPixelsPtr(), handCursorImage_->getSize(), sf::Vector2u{0U, 0U});
        window_.setMouseCursor(*arrowCursor_);
    }

    void ExplorationSession::createParty()
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

} // namespace soma::presentation::detail

namespace soma::presentation
{
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
            detail::ExplorationSession{battle, economy, world, rules, level, campaignState, economyService, randomSeed, std::move(authoredLevel)}
                .run());
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
        detail::ExplorationSession session{battle,
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
