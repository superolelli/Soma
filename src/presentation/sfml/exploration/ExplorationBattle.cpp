#include "presentation/sfml/battle/BattleActionPresentation.hpp"
#include "presentation/sfml/exploration/ExplorationSessionController.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

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
        request.playerHealth = progress_.playerHealth;
        request.playerFatigue = progress_.playerFatigue;
        request.playerBuffs = progress_.playerBuffs;
        request.bossCombatant = worldEncounter.bossCombatant;
        request.bossHealthAndDamageMultiplier = campaignBattle_.bossHealthAndDamageMultiplier;
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
            request.preparedCombatantViews = std::move(preparedBattleViews_);
            preparedEncounter_.reset();
        }
        if (worldEncounter.introCue)
        {
            request.introVideo = world_.assets.at(*worldEncounter.introCue);
        }
        request.returnWhenResolved = true;
        request.flow = flow_;
        auto outcome = runBattleScreenSession(window_, campaignBattle_, economy_, rules_, campaignState_, std::move(request), &videoPlayer_);
        if (outcome.result == BattleScreenResult::saveAndMainRoom || outcome.result == BattleScreenResult::abandonAndMainRoom)
        {
            campaignState_ = campaignBeforeBattle;
            if (levelRunBeforeBattle_)
            {
                levelRun_ = *levelRunBeforeBattle_;
            }
            result_ = outcome.result == BattleScreenResult::saveAndMainRoom ? game::LevelSessionResult::saveAndMainMenu
                                                                            : game::LevelSessionResult::abandonAndMainMenu;
            requestSessionClose();
            return false;
        }
        if (outcome.result == BattleScreenResult::windowClosed)
        {
            return false;
        }
        music_.resumeAfterBattle();
        presentBattleOutcome(worldEncounter, level, std::move(outcome));
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
        const auto& worldEncounter = world_.encounters.at(*encounter);
        simulation::BattleSimulation preparationBattle{
            campaignBattle_,
            createBattleSpawns(battleEncounter,
                               enemies,
                               progress_.playerHealth,
                               progress_.playerFatigue,
                               progress_.playerBuffs,
                               worldEncounter.bossCombatant,
                               campaignBattle_.bossHealthAndDamageMultiplier),
            preparationRandom};
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

    void ExplorationSession::presentBattleOutcome(const content::EncounterDefinition& encounter,
                                                  const content::LevelDefinition& level,
                                                  BattleScreenOutcome outcome)
    {
        const auto followUp = game::ExplorationOutcomeService::applyEncounter(
            {outcome.result == BattleScreenResult::playersWon,
             std::move(outcome.playerHealth),
             std::move(outcome.playerFatigue),
             std::move(outcome.usedPlayerAbilities),
             std::move(outcome.playerBuffs)},
            static_cast<int>(encounter.combatants.size()),
            encounter.boss,
            progress_);
        if (followUp == game::EncounterFollowUp::battleRewards)
        {
            openBattleRewards(encounter, level);
            return;
        }
        if (followUp == game::EncounterFollowUp::bossRewards)
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
            return;
        }
        openLevelResult(level, false);
    }

    void ExplorationSession::openBattleRewards(const content::EncounterDefinition& encounter, const content::LevelDefinition& level)
    {
        const simulation::RuleLootContext ruleContext{simulation::RuleRewardSource::battle, level.campaign, level.id, encounter.combatants};
        auto rewards = lootGenerator_.generate(encounter.loot, level.campaign, level.difficulty, ruleContext);
        lootDialog_ = std::make_unique<LootDialog>(
            world_, economy_, rules_, economyTextures_, sounds_, economyService_, ruleService_, "Kampf gewonnen!", std::nullopt, std::move(rewards));
    }

    void ExplorationSession::openLevelResult(const content::LevelDefinition& level, bool completed)
    {
        result_ = completed ? game::LevelSessionResult::completed : game::LevelSessionResult::defeated;
        auto result = game::ExplorationOutcomeService::completeLevel(level,
                                                                      completed,
                                                                      progress_,
                                                                      initiallyUnlockedItems_,
                                                                      initiallyDiscoveredRuleTypes_,
                                                                      campaignState_,
                                                                      economyService_,
                                                                      economy_,
                                                                      rules_);
        if (!result.hasValue())
        {
            throw std::runtime_error(result.error());
        }
        auto summary = std::move(result.value());
        levelResultDialog_ = std::make_unique<LevelResultDialog>(world_,
                                                                 economy_,
                                                                 rules_,
                                                                 economyTextures_,
                                                                 level.campaign,
                                                                 completed,
                                                                 std::move(summary.rewards),
                                                                 std::move(summary.newlyUnlockedItems),
                                                                 std::move(summary.newlyDiscoveredRuleTypes));
    }

} // namespace soma::presentation::detail
