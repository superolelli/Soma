#include "presentation/sfml/BattleScreenSession.hpp"

#include "gameplay/random/SeededRandom.hpp"
#include "presentation/sfml/ActiveRulesPanel.hpp"
#include "presentation/sfml/AudioSettings.hpp"
#include "presentation/sfml/BattleActionStarter.hpp"
#include "presentation/sfml/BattleActionTimeline.hpp"
#include "presentation/sfml/BattleInteractionController.hpp"
#include "presentation/sfml/BattleLaunchOptions.hpp"
#include "presentation/sfml/BattlePlaybackSettings.hpp"
#include "presentation/sfml/BattleSceneBlur.hpp"
#include "presentation/sfml/BattleScreenRenderer.hpp"
#include "presentation/sfml/BattleScreenView.hpp"
#include "presentation/sfml/BattleSoundPlayer.hpp"
#include "presentation/sfml/BattleTransitionPlayer.hpp"
#include "presentation/sfml/EconomyTextures.hpp"
#include "presentation/sfml/PauseMenu.hpp"
#include "presentation/sfml/SpriterAnimationPlayer.hpp"
#include "presentation/sfml/UiCursor.hpp"
#include "presentation/video/IVideoPlayer.hpp"
#include "simulation/battle/BattleSimulation.hpp"
#include "simulation/battle/EnemyPolicy.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <memory>
#include <optional>

namespace soma::presentation
{
    namespace
    {
        std::vector<simulation::BattleSpawn> buildBattleSpawns(
            const content::BattleContent::Encounter& encounter,
            const std::vector<gameplay::ContentDefinitionId>& enemies,
            const std::unordered_map<gameplay::ContentDefinitionId, int>& playerHealth,
            const std::unordered_map<gameplay::ContentDefinitionId, int>& playerFatigue,
            const std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>>& playerBuffs,
            const std::optional<gameplay::ContentDefinitionId>& bossCombatant,
            double bossAttributeMultiplier)
        {
            std::vector<simulation::BattleSpawn> spawns;
            for (const auto& [combatant, slot] : encounter.combatants)
            {
                if (enemies.empty() || slot < 4)
                {
                    const auto health = playerHealth.find(combatant);
                    const auto fatigue = playerFatigue.find(combatant);
                    const auto buffs = playerBuffs.find(combatant);
                    spawns.push_back({combatant,
                                      slot,
                                      health == playerHealth.end() ? std::nullopt : std::optional<int>{health->second},
                                      buffs == playerBuffs.end() ? std::vector<simulation::ActiveBuff>{} : buffs->second,
                                      fatigue == playerFatigue.end() ? 0 : fatigue->second});
                }
            }
            auto bossSpawned = false;
            for (std::size_t index = 0; index < enemies.size(); ++index)
            {
                const auto isBoss = !bossSpawned && bossCombatant && enemies.at(index) == *bossCombatant;
                spawns.push_back({enemies.at(index),
                                  static_cast<int>(index) + 4,
                                  std::nullopt,
                                  {},
                                  0,
                                  isBoss ? bossAttributeMultiplier : 1.0});
                bossSpawned = bossSpawned || isBoss;
            }
            return spawns;
        }

        void updateAnimations(sf::Time elapsed, CombatantViews& views, IAnimationPlayer& effect, std::vector<FloatingText>& floatingTexts)
        {
            for (auto& [id, view] : views)
            {
                static_cast<void>(id);
                view.animation->update(static_cast<float>(elapsed.asMilliseconds()));
                updateCombatantBounds(view);
                if (view.displayedHealth != static_cast<float>(view.targetHealth))
                {
                    constexpr float transitionSeconds = 0.7F;
                    view.healthAnimationElapsed = std::min(transitionSeconds, view.healthAnimationElapsed + elapsed.asSeconds());
                    const auto progress = view.healthAnimationElapsed / transitionSeconds;
                    view.displayedHealth = view.healthAnimationStart + (static_cast<float>(view.targetHealth) - view.healthAnimationStart) * progress;
                }
                if (view.displayedFatigue != static_cast<float>(view.targetFatigue))
                {
                    constexpr float transitionSeconds = 0.7F;
                    view.fatigueAnimationElapsed = std::min(transitionSeconds, view.fatigueAnimationElapsed + elapsed.asSeconds());
                    const auto progress = view.fatigueAnimationElapsed / transitionSeconds;
                    view.displayedFatigue =
                        view.fatigueAnimationStart + (static_cast<float>(view.targetFatigue) - view.fatigueAnimationStart) * progress;
                }
            }
            effect.update(static_cast<float>(elapsed.asMilliseconds()));
            for (auto& floating : floatingTexts)
            {
                floating.elapsed += elapsed.asSeconds();
            }
            std::erase_if(floatingTexts, [](const auto& floating) {
                return floating.elapsed >= 1.0F;
            });
        }

        std::unordered_map<gameplay::ContentDefinitionId, int> collectPlayerHealth(const simulation::BattleSimulation& battle)
        {
            std::unordered_map<gameplay::ContentDefinitionId, int> result;
            for (const auto& combatant : battle.combatants())
            {
                if (combatant.team == gameplay::Team::players)
                {
                    result.emplace(combatant.definition, combatant.health);
                }
            }
            return result;
        }

        std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> collectPlayerBuffs(
            const simulation::BattleSimulation& battle)
        {
            std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> result;
            for (const auto& combatant : battle.combatants())
            {
                if (combatant.team == gameplay::Team::players && !combatant.buffs.empty())
                {
                    result.emplace(combatant.definition, combatant.buffs);
                }
            }
            return result;
        }

        std::unordered_map<gameplay::ContentDefinitionId, int> collectPlayerFatigue(const simulation::BattleSimulation& battle)
        {
            std::unordered_map<gameplay::ContentDefinitionId, int> result;
            for (const auto& combatant : battle.combatants())
            {
                if (combatant.team == gameplay::Team::players)
                {
                    result.emplace(combatant.definition, combatant.fatigue);
                }
            }
            return result;
        }

        BattleScreenOutcome outcome(BattleScreenResult result, const simulation::BattleSimulation& battle)
        {
            return {result, collectPlayerHealth(battle), collectPlayerFatigue(battle), battle.usedPlayerAbilities(), collectPlayerBuffs(battle)};
        }
    } // namespace

    std::vector<simulation::BattleSpawn> createBattleSpawns(
        const content::BattleContent::Encounter& encounter,
        const std::vector<gameplay::ContentDefinitionId>& enemies,
        const std::unordered_map<gameplay::ContentDefinitionId, int>& playerHealth,
        const std::unordered_map<gameplay::ContentDefinitionId, int>& playerFatigue,
        const std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>>& playerBuffs,
        const std::optional<gameplay::ContentDefinitionId>& bossCombatant,
        double bossAttributeMultiplier)
    {
        return buildBattleSpawns(encounter, enemies, playerHealth, playerFatigue, playerBuffs, bossCombatant, bossAttributeMultiplier);
    }

    BattleScreenOutcome runBattleScreenSession(sf::RenderWindow& window,
                                               const content::BattleContent& battleContent,
                                               const content::EconomyContent& economyContent,
                                               const content::RuleContent& ruleContent,
                                               game::CampaignState& campaignState,
                                               const BattleScreenRequest& request,
                                               IVideoPlayer* videoPlayer)
    {
        const auto& layout = battleContent.layout;
        const sf::Vector2u referenceSize{static_cast<unsigned int>(layout.referenceWidth), static_cast<unsigned int>(layout.referenceHeight)};
        window.setView(sf::View{sf::FloatRect{{0.0F, 0.0F}, {static_cast<float>(referenceSize.x), static_cast<float>(referenceSize.y)}}});

        const sf::Font interfaceFont{battleContent.assets.at("interface_font")};
        const sf::Font notificationFont{battleContent.assets.at("notification_font")};
        std::optional<BattleTextures> ownedTextures;
        if (request.preparedTextures == nullptr)
        {
            ownedTextures.emplace(battleContent);
        }
        const auto& textures = request.preparedTextures != nullptr ? *request.preparedTextures : *ownedTextures;
        const EconomyTextures economyTextures{economyContent};
        const auto skipTurnFrameWidth = textures.skipTurn.getSize().x / 2U;
        const sf::Vector2f skipTurnPosition{
            static_cast<float>(layout.abilityPanelX) + static_cast<float>(textures.abilityPanel.getSize().x),
            static_cast<float>(layout.abilityPanelY) +
                (static_cast<float>(textures.abilityPanel.getSize().y) - static_cast<float>(textures.skipTurn.getSize().y)) / 2.0F};
        const sf::FloatRect skipTurnBounds{skipTurnPosition,
                                           {static_cast<float>(skipTurnFrameWidth), static_cast<float>(textures.skipTurn.getSize().y)}};

        std::vector<sf::Texture> backgroundTextures;
        backgroundTextures.reserve(request.backgroundLayers.size() + 1U);
        for (const auto& layer : request.backgroundLayers)
        {
            backgroundTextures.emplace_back(layer.texture);
        }
        backgroundTextures.emplace_back(request.background);
        std::vector<sf::Sprite> backgrounds;
        backgrounds.reserve(backgroundTextures.size());
        for (std::size_t index = 0; index < request.backgroundLayers.size(); ++index)
        {
            backgrounds.emplace_back(backgroundTextures.at(index));
            backgrounds.back().setPosition(request.backgroundLayers.at(index).position);
        }
        backgrounds.emplace_back(backgroundTextures.back());
        ParticleSystem particles{request.particles, request.randomSeed + 3U};
        particles.update(sf::Time::Zero, {{0.0F, 0.0F}, {static_cast<float>(referenceSize.x), static_cast<float>(referenceSize.y)}});
        BattleSceneBlur sceneBlur{window, referenceSize, battleContent.assets.at("blur_shader")};
        gameplay::SeededRandom random{request.randomSeed};
        simulation::BattleSimulation battle{
            battleContent,
            createBattleSpawns(request.encounter,
                               request.enemies,
                               request.playerHealth,
                               request.playerFatigue,
                               request.playerBuffs,
                               request.bossCombatant,
                               request.bossAttributeMultiplier),
            random,
            &ruleContent,
            &campaignState.rules()};
        simulation::EnemyPolicy enemyPolicy{random};
        auto views = request.preparedCombatantViews != nullptr ? std::move(*request.preparedCombatantViews)
                                                               : createCombatantViews(window, battleContent, battle);
        for (const auto& state : battle.combatants())
        {
            auto& view = views.at(state.id);
            view.displayedHealth = static_cast<float>(state.health);
            view.healthAnimationStart = static_cast<float>(state.health);
            view.targetHealth = state.health;
            view.maximumHealth = state.attributes.maximumHealth;
            view.displayedFatigue = static_cast<float>(state.fatigue);
            view.fatigueAnimationStart = static_cast<float>(state.fatigue);
            view.targetFatigue = state.fatigue;
            view.fatigueAnimationElapsed = 0.7F;
        }
        restoreCombatants(views);
        positionEnemyViews(views, battle);

        std::unique_ptr<SpriterAnimationPlayer> ownedEffect;
        auto* effect = request.preparedEffectAnimation;
        if (effect == nullptr)
        {
            ownedEffect = std::make_unique<SpriterAnimationPlayer>(window, battleContent.assets.at("ability_effect_model"), "ability_effects");
            effect = ownedEffect.get();
        }
        effect->play("empty", 1.0F);

        std::optional<sf::Music> music;
        if (!request.music.empty())
        {
            music.emplace(request.music);
            music->setLooping(true);
            music->setRelativeToListener(true);
            music->setVolume(AudioSettings::music(30.0F));
        }

        std::optional<PresentedAction> action;
        std::vector<FloatingText> floatingTexts;
        const auto initialCombatants = battle.combatants();
        auto initialEvents = battle.begin();
        const auto initialItemActivation = std::ranges::find_if(initialEvents, [](const auto& event) {
            return std::holds_alternative<gameplay::ItemActivated>(event);
        });
        if (initialItemActivation == initialEvents.end())
        {
            applyPresentedEvents(initialEvents, views, floatingTexts);
        }
        else
        {
            std::vector<gameplay::GameplayEvent> immediateEvents{initialEvents.begin(), initialItemActivation};
            applyPresentedEvents(immediateEvents, views, floatingTexts);
            const auto& activation = std::get<gameplay::ItemActivated>(*initialItemActivation);
            const auto* carrier = battle.find(activation.carrier);
            const auto& ability = battleContent.combatants.at(carrier->definition).abilities.front();
            std::vector<gameplay::GameplayEvent> itemEvents{initialItemActivation, initialEvents.end()};
            action = PresentedAction{{activation.carrier, ability, activation.carrier},
                                     std::move(itemEvents),
                                     {},
                                     {},
                                     {},
                                     initialCombatants,
                                     {},
                                     ActionStage::itemInterruption};
        }
        applyPersistentAnimations(views, battle);
        sf::Clock frameClock;
        BattleSoundPlayer soundPlayer{battleContent};
        const BattleActionTimeline actionTimeline{soundPlayer};
        BattleActionStarter actionStarter{window, battleContent, battle, views, *effect, action, floatingTexts};
        auto launchOptions = request.developmentLaunchOptions.value_or(BattleLaunchOptions{});
        launchOptions.enemies = request.enemies;
        BattleInteractionController interaction{
            window, battleContent, battle, enemyPolicy, actionStarter, launchOptions, views, action, skipTurnBounds};
        ResourceCounter resources{economyContent, campaignState};
        ActiveRulesPanel activeRules{ruleContent, campaignState.rules(), interfaceFont};
        BattleScreenRenderer renderer{window,
                                      battleContent,
                                      battle,
                                      views,
                                      *effect,
                                      textures,
                                      interfaceFont,
                                      notificationFont,
                                      backgrounds,
                                      particles,
                                      sceneBlur,
                                      action,
                                      floatingTexts,
                                      interaction,
                                      resources,
                                      skipTurnPosition,
                                      skipTurnBounds,
                                      skipTurnFrameWidth};
        if (request.onBattleTransition)
        {
            request.onBattleTransition();
        }
        if (request.transition && !BattleTransitionPlayer{}.play(
                                      window,
                                      *request.transition,
                                      [&]() {
                                          renderer.draw(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
                                      },
                                      request.preparedTransitionAnimation))
        {
            return outcome(BattleScreenResult::windowClosed, battle);
        }
        if (request.introVideo && videoPlayer && !videoPlayer->play(window, *request.introVideo))
        {
            return outcome(BattleScreenResult::windowClosed, battle);
        }
        if (request.onBattleReady)
        {
            request.onBattleReady();
        }
        if (music)
        {
            music->play();
        }

        // Transition and intro playback are synchronous. Do not feed their
        // duration into the first simulation frame when the battle becomes live.
        frameClock.restart();

        std::unique_ptr<PauseMenu> pauseMenu;
        if (request.flow)
        {
            pauseMenu = std::make_unique<PauseMenu>(*request.flow, PauseMenuContext::level);
        }
        std::optional<BattleScreenResult> pauseResult;

        while (window.isOpen())
        {
            const auto elapsed = frameClock.restart();
            if (!pauseMenu || !pauseMenu->isOpen())
            {
                interaction.prepareTurn();
            }
            interaction.processEvents([&](const sf::Event& event) {
                if (pauseMenu)
                {
                    const auto menuAction = pauseMenu->handleEvent(event, window);
                    if (menuAction == PauseMenuAction::saveAndMainMenu)
                    {
                        pauseResult = BattleScreenResult::saveAndMainRoom;
                    }
                    if (menuAction == PauseMenuAction::abandonAndMainMenu)
                    {
                        pauseResult = BattleScreenResult::abandonAndMainRoom;
                    }
                    if (menuAction.has_value() || pauseMenu->isOpen())
                    {
                        return true;
                    }
                }
                const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
                return click != nullptr && click->button == sf::Mouse::Button::Left &&
                       activeRules.handleClick(window.mapPixelToCoords(click->position));
            });
            if (pauseResult)
            {
                return outcome(*pauseResult, battle);
            }
            if (!pauseMenu || !pauseMenu->isOpen())
            {
                particles.update(elapsed, {{0.0F, 0.0F}, {static_cast<float>(referenceSize.x), static_cast<float>(referenceSize.y)}});
                const auto playbackElapsed = sf::seconds(elapsed.asSeconds() * BattlePlaybackSettings::timeScale());
                actionTimeline.update(playbackElapsed.asSeconds(), action, views, *effect, battle, floatingTexts);
                updateAnimations(playbackElapsed, views, *effect, floatingTexts);
            }
            const auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            if (pauseMenu && pauseMenu->isOpen())
            {
                renderer.draw(mouse);
                activeRules.render(window, mouse);
                pauseMenu->render(window, mouse);
                window.display();
            }
            else
            {
                setUiCursor(window, interaction.clickableAt(mouse) || activeRules.clickableAt(mouse));
                renderer.draw(mouse);
                activeRules.render(window, mouse);
                window.display();
            }
            if (request.returnWhenResolved && battle.finished() && !action.has_value())
            {
                const auto result = battle.winner() == gameplay::Team::players ? BattleScreenResult::playersWon : BattleScreenResult::enemiesWon;
                return outcome(result, battle);
            }
        }
        return outcome(BattleScreenResult::windowClosed, battle);
    }
} // namespace soma::presentation
