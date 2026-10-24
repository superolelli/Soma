#include "presentation/sfml/BattleActionPresentation.hpp"
#include "presentation/sfml/ExplorationSessionController.hpp"

#include <cmath>
#include <limits>
#include <ranges>

namespace soma::presentation::detail
{
    bool ExplorationSession::runBattle()
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
        request.particles = resolveParticleEmitters(world_.campaignPresentation.at(level.campaign).particles, world_.assets);
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
        const auto outcome = runBattleScreenSession(window_, campaignBattle_, economy_, rules_, campaignState_, request, &videoPlayer_);
        if (outcome.result == BattleScreenResult::saveAndMainRoom || outcome.result == BattleScreenResult::abandonAndMainRoom)
        {
            campaignState_ = campaignBeforeBattle;
            if (levelRunBeforeBattle_)
            {
                levelRun_ = *levelRunBeforeBattle_;
            }
            result_ =
                outcome.result == BattleScreenResult::saveAndMainRoom ? ExplorationResult::saveAndMainMenu : ExplorationResult::abandonAndMainMenu;
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

    void ExplorationSession::prepareUpcomingBattleViews()
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

    void ExplorationSession::appendBattleParallaxLayers(BattleScreenRequest& request) const
    {
        const auto& parallax = levelRun_.parallax();
        const std::array fourthLayer{parallax.fourthLayer};
        appendBattleParallaxLayer(request, fourthLayer, parallax.fourthLayerDivisor);
        appendBattleParallaxLayer(request, parallax.thirdLayerTiles, parallax.thirdLayerDivisor);
        appendBattleParallaxLayer(request, parallax.secondLayerTiles, parallax.secondLayerDivisor);
    }

    void ExplorationSession::appendBattleParallaxLayer(BattleScreenRequest& request,
                                                       std::span<const gameplay::AssetId> tiles,
                                                       int scrollDivisor) const
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

    void ExplorationSession::applyBattleOutcome(const content::EncounterDefinition& encounter,
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
            const simulation::RuleLootContext ruleContext{simulation::RuleRewardSource::battle, level.campaign, level.id, encounter.combatants};
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

    void ExplorationSession::openBattleRewards(const content::EncounterDefinition& encounter, const content::LevelDefinition& level)
    {
        const simulation::RuleLootContext ruleContext{simulation::RuleRewardSource::battle, level.campaign, level.id, encounter.combatants};
        auto rewards = lootGenerator_.generate(encounter.rewardTable, level.campaign, level.difficulty, ruleContext);
        lootDialog_ = std::make_unique<LootDialog>(
            world_, economy_, rules_, economyTextures_, sounds_, economyService_, ruleService_, "Kampf gewonnen!", std::nullopt, std::move(rewards));
    }

    void ExplorationSession::openLevelResult(const content::LevelDefinition& level, bool completed)
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

    std::vector<gameplay::ItemId> ExplorationSession::newlyUnlockedItems() const
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

    std::vector<content::RuleFragmentType> ExplorationSession::newlyDiscoveredRuleTypes() const
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

    void ExplorationSession::grantLevelRewards(const simulation::LevelRewards& rewards)
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

} // namespace soma::presentation::detail
