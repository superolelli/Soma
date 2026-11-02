#include "presentation/sfml/tutorial/InitialTutorialSession.hpp"

#include "game/CampaignBattleContentBuilder.hpp"
#include "game/EconomyService.hpp"
#include "game/RuleBuilderService.hpp"
#include "presentation/sfml/battle/BattleScreenSession.hpp"
#include "presentation/sfml/common/SpriterAnimationPlayer.hpp"
#include "presentation/sfml/common/TutorialOverlay.hpp"
#include "presentation/sfml/common/NinePatch.hpp"
#include "presentation/sfml/economy/EconomySoundPlayer.hpp"
#include "presentation/sfml/economy/EconomyTextures.hpp"
#include "presentation/sfml/exploration/LootDialog.hpp"
#include "simulation/rewards/LootGenerator.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <random>
#include <sstream>
#include <thread>

namespace soma::presentation
{
    namespace
    {
        constexpr float roomWidth = 1920.0F;
        const std::array playerIds{gameplay::ContentDefinitionId{"player.simon"},
                                   gameplay::ContentDefinitionId{"player.ole"},
                                   gameplay::ContentDefinitionId{"player.anna"},
                                   gameplay::ContentDefinitionId{"player.markus"}};

        sf::Vector2f vector(const content::Point& value)
        {
            return {static_cast<float>(value.x), static_cast<float>(value.y)};
        }
    } // namespace

    InitialTutorialSession::InitialTutorialSession(sf::RenderWindow& window,
                                                   const content::BattleContent& battle,
                                                   const content::EconomyContent& economy,
                                                   const content::WorldContent& world,
                                                   const content::GameFlowContent& flow,
                                                   const content::RuleContent& rules,
                                                   const content::TutorialContent& tutorial,
                                                   game::CampaignState& campaign)
        : window_(window)
        , battle_(battle)
        , economy_(economy)
        , world_(world)
        , flow_(flow)
        , rules_(rules)
        , tutorial_(tutorial)
        , campaign_(campaign)
        , worldView_(sf::FloatRect{{0.0F, 0.0F}, {1920.0F, 1080.0F}})
        , hudView_(sf::FloatRect{{0.0F, 0.0F}, {1920.0F, 1080.0F}})
        , backgrounds_{sf::Texture{tutorial.firstBackground}, sf::Texture{tutorial.battleBackground}, sf::Texture{tutorial.endBackground}}
        , chestTexture_(world.assets.at(world.lootables.at(gameplay::LootableId{tutorial.lootable}).sprite))
        , doorTexture_(tutorial.door)
        , speechBubbleTexture_(world.assets.at(world.presentation.speechBubbles.texture))
        , font_(tutorial.font)
    {
        createParty();
    }

    InitialTutorialResult InitialTutorialSession::run()
    {
        window_.setView(hudView_);
        if (!showMessage(tutorial_.initialMessages.at("navigation")))
        {
            return InitialTutorialResult::windowClosed;
        }

        sf::Clock clock;
        while (window_.isOpen())
        {
            while (const auto event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window_.close();
                    return InitialTutorialResult::windowClosed;
                }
                if (const auto* click = event->getIf<sf::Event::MouseButtonReleased>();
                    click && click->button == sf::Mouse::Button::Left && !chestOpened_ &&
                    chestBounds().contains(window_.mapPixelToCoords(click->position, worldView_)))
                {
                    if (!openChest()) return InitialTutorialResult::windowClosed;
                }
            }

            if (!lootHintShown_ && viewLeft_ > 80.0F)
            {
                lootHintShown_ = true;
                const auto& lootable = world_.lootables.at(gameplay::LootableId{tutorial_.lootable});
                const content::Rectangle highlight{static_cast<int>(static_cast<float>(lootable.position.x) - viewLeft_),
                                                   lootable.position.y,
                                                   static_cast<int>(chestTexture_.getSize().x / 2U),
                                                   static_cast<int>(chestTexture_.getSize().y)};
                if (!showMessage(tutorial_.initialMessages.at("lootable"), {highlight})) return InitialTutorialResult::windowClosed;
            }
            if (viewLeft_ >= roomWidth - 40.0F && !chestOpened_)
            {
                viewLeft_ = roomWidth - 40.0F;
            }
            if (viewLeft_ >= roomWidth && !battleCompleted_)
            {
                if (!runBattle()) return InitialTutorialResult::windowClosed;
                battleCompleted_ = true;
                if (!showBattleRewards()) return InitialTutorialResult::windowClosed;
                if (!showMessage(tutorial_.initialMessages.at("battle_loot"))) return InitialTutorialResult::windowClosed;
                clock.restart();
            }
            if (battleCompleted_ && viewLeft_ >= roomWidth * 2.0F - 30.0F)
            {
                if (!playEnding()) return InitialTutorialResult::windowClosed;
                return InitialTutorialResult::completed;
            }

            update(clock.restart(), movementDirection());
            drawFrame();
            window_.display();
        }
        return InitialTutorialResult::windowClosed;
    }

    void InitialTutorialSession::createParty()
    {
        for (std::size_t index = 0; index < playerIds.size(); ++index)
        {
            const auto& presentation = battle_.presentation.at(playerIds.at(index));
            auto animation = std::make_unique<SpriterAnimationPlayer>(window_, presentation.model, presentation.entity);
            animation->setScale(static_cast<float>(world_.presentation.partyScale), static_cast<float>(world_.presentation.partyScale));
            animation->setPosition(static_cast<float>(world_.presentation.partyPositions.at(index).x),
                                   static_cast<float>(world_.presentation.partyPositions.at(index).y));
            animation->play(world_.presentation.idleAnimation, static_cast<float>(world_.presentation.idleAnimationSpeed));
            party_.push_back(std::move(animation));
        }
    }

    float InitialTutorialSession::movementDirection() const
    {
        const auto movingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
        const auto movingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
        const auto keyboard = (movingRight ? 1.0F : 0.0F) - (movingLeft ? 1.0F : 0.0F);
        if (keyboard != 0.0F) return keyboard;
        const auto mouse = sf::Mouse::getPosition(window_).x;
        if (mouse < 60) return -1.0F;
        if (mouse > static_cast<int>(window_.getSize().x) - 60) return 1.0F;
        return 0.0F;
    }

    void InitialTutorialSession::update(sf::Time elapsed, float direction)
    {
        viewLeft_ = std::clamp(viewLeft_ + direction * static_cast<float>(tutorial_.movementSpeed) * elapsed.asSeconds(), 0.0F, roomWidth * 2.0F);
        worldView_.setCenter({viewLeft_ + 960.0F, 540.0F});
        const auto nowWalking = direction != 0.0F;
        if (walking_ != nowWalking)
        {
            walking_ = nowWalking;
            for (auto& player : party_)
            {
                player->play(walking_ ? world_.presentation.walkAnimation : world_.presentation.idleAnimation,
                             static_cast<float>(walking_ ? world_.presentation.walkAnimationSpeed :
                                                           world_.presentation.idleAnimationSpeed));
            }
        }
        for (std::size_t index = 0; index < party_.size(); ++index)
        {
            party_.at(index)->setPosition(viewLeft_ + static_cast<float>(world_.presentation.partyPositions.at(index).x),
                                          static_cast<float>(world_.presentation.partyPositions.at(index).y));
            party_.at(index)->update(static_cast<float>(elapsed.asMilliseconds()));
        }
    }

    void InitialTutorialSession::drawFrame()
    {
        window_.setView(worldView_);
        window_.clear(sf::Color::Black);
        for (std::size_t index = 0; index < backgrounds_.size(); ++index)
        {
            sf::Sprite background{backgrounds_.at(index)};
            background.setPosition({roomWidth * static_cast<float>(index), 0.0F});
            window_.draw(background);
        }
        drawChest();
        drawDoor();
        for (std::size_t index = 0; index < party_.size(); ++index)
        {
            if (!entered_.at(index)) party_.at(index)->render();
        }
        window_.setView(hudView_);
    }

    void InitialTutorialSession::drawChest()
    {
        sf::Sprite chest{chestTexture_};
        const auto& definition = world_.lootables.at(gameplay::LootableId{tutorial_.lootable});
        const auto frameWidth = static_cast<int>(chestTexture_.getSize().x / 2U);
        chest.setTextureRect({{chestOpened_ ? frameWidth : 0, 0}, {frameWidth, static_cast<int>(chestTexture_.getSize().y)}});
        chest.setPosition(vector(definition.position));
        window_.draw(chest);
    }

    void InitialTutorialSession::drawDoor()
    {
        sf::Sprite door{doorTexture_};
        door.setPosition({roomWidth * 2.0F + static_cast<float>(tutorial_.doorPosition.x) + doorOffset_,
                          static_cast<float>(tutorial_.doorPosition.y)});
        window_.draw(door);
    }

    sf::FloatRect InitialTutorialSession::chestBounds() const
    {
        const auto& definition = world_.lootables.at(gameplay::LootableId{tutorial_.lootable});
        return {{static_cast<float>(definition.position.x + definition.hitbox.x),
                 static_cast<float>(definition.position.y + definition.hitbox.y)},
                {static_cast<float>(definition.hitbox.width), static_cast<float>(definition.hitbox.height)}};
    }

    bool InitialTutorialSession::showMessage(std::string_view id, std::vector<content::Rectangle> highlights)
    {
        window_.setView(hudView_);
        auto message = tutorial_.messages.at(std::string{id});
        if (!highlights.empty()) message.highlights = std::move(highlights);
        return TutorialOverlay{window_, font_}.show(message, [this]() { drawFrame(); });
    }

    bool InitialTutorialSession::showMessages(const std::vector<std::string>& ids)
    {
        for (const auto& id : ids)
        {
            if (!showMessage(id)) return false;
        }
        return true;
    }

    bool InitialTutorialSession::openChest()
    {
        game::EconomyService economyService{campaign_};
        game::RuleBuilderService ruleService{campaign_, rules_};
        EconomyTextures textures{economy_, battle_};
        EconomySoundPlayer sounds;
        simulation::LootBundle loot;
        loot.items.push_back({{gameplay::ItemId{tutorial_.rewardItem}, {128, 0, 0}}, 1});
        LootDialog dialog{world_,
                          economy_,
                          rules_,
                          textures,
                          sounds,
                          economyService,
                          ruleService,
                          "Gefunden!",
                          world_.assets.at(world_.lootables.at(gameplay::LootableId{tutorial_.lootable}).sound),
                          std::move(loot)};
        while (window_.isOpen() && !dialog.closed())
        {
            while (const auto event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>()) window_.close();
                if (const auto* click = event->getIf<sf::Event::MouseButtonReleased>(); click && click->button == sf::Mouse::Button::Left)
                    dialog.handleClick(window_.mapPixelToCoords(click->position, hudView_));
            }
            drawFrame();
            dialog.render(window_, window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_));
            window_.display();
        }
        chestOpened_ = campaign_.itemUnlocked(gameplay::ItemId{tutorial_.rewardItem});
        return window_.isOpen();
    }

    bool InitialTutorialSession::runBattle()
    {
        const auto campaignBattle = game::CampaignBattleContentBuilder{}.build(battle_, economy_, campaign_);
        auto explanationsShown = false;
        while (window_.isOpen())
        {
            BattleScreenRequest request;
            request.encounter = campaignBattle.encounters.at(gameplay::ContentDefinitionId{"encounter.bang.default"});
            request.enemies = {gameplay::ContentDefinitionId{"enemy.abtruenniger"}, gameplay::ContentDefinitionId{"enemy.abtruenniger"}};
            request.background = tutorial_.battleBackground;
            request.music = request.encounter.music;
            request.randomSeed = std::random_device{}();
            request.returnWhenResolved = true;
            request.flow = &flow_;
            const auto& transition = world_.presentation.battleTransition;
            request.transition = BattleTransitionRequest{world_.assets.at(transition.model),
                                                         transition.entity,
                                                         transition.animation,
                                                         vector(transition.position),
                                                         static_cast<float>(transition.scale),
                                                         static_cast<float>(transition.delayAfterSeconds)};
            if (!explanationsShown)
            {
                request.beforeFirstTurn = [this, &explanationsShown](const std::function<void()>& drawBattle) {
                    TutorialOverlay overlay{window_, font_};
                    for (const auto& id : tutorial_.battleMessages)
                    {
                        if (!overlay.show(tutorial_.messages.at(id), drawBattle)) return false;
                    }
                    explanationsShown = true;
                    return true;
                };
            }
            const auto outcome = runBattleScreenSession(window_, campaignBattle, economy_, rules_, campaign_, std::move(request));
            if (outcome.result == BattleScreenResult::playersWon) return true;
            if (outcome.result != BattleScreenResult::enemiesWon) return false;
            if (!showMessage(tutorial_.initialMessages.at("retry_battle"))) return false;
        }
        return false;
    }

    bool InitialTutorialSession::showBattleRewards()
    {
        game::EconomyService economyService{campaign_};
        const auto cardsBefore = campaign_.currency(gameplay::Currency::cards);
        const auto diceBefore = campaign_.currency(gameplay::Currency::dice);
        game::RuleBuilderService ruleService{campaign_, rules_};
        EconomyTextures textures{economy_, battle_};
        EconomySoundPlayer sounds;
        simulation::LootBundle loot;
        loot.items.push_back({{gameplay::ItemId{"currency.cards"}, {}}, tutorial_.rewardCards});
        loot.items.push_back({{gameplay::ItemId{"currency.dice"}, {}}, tutorial_.rewardDice});
        LootDialog dialog{world_, economy_, rules_, textures, sounds, economyService, ruleService, "Kampf gewonnen!", std::nullopt, std::move(loot)};
        while (window_.isOpen() && !dialog.closed())
        {
            while (const auto event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>()) window_.close();
                if (const auto* click = event->getIf<sf::Event::MouseButtonReleased>(); click && click->button == sf::Mouse::Button::Left)
                    dialog.handleClick(window_.mapPixelToCoords(click->position, hudView_));
            }
            drawFrame();
            dialog.render(window_, window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_));
            window_.display();
        }
        const auto missingCards = tutorial_.rewardCards - (campaign_.currency(gameplay::Currency::cards) - cardsBefore);
        const auto missingDice = tutorial_.rewardDice - (campaign_.currency(gameplay::Currency::dice) - diceBefore);
        if (missingCards > 0)
            static_cast<void>(economyService.grant(economy_.items.at(gameplay::ItemId{"currency.cards"}), missingCards));
        if (missingDice > 0)
            static_cast<void>(economyService.grant(economy_.items.at(gameplay::ItemId{"currency.dice"}), missingDice));
        return window_.isOpen();
    }

    bool InitialTutorialSession::playEnding()
    {
        const auto dialogueBeforeDoor = std::min<std::size_t>(3U, tutorial_.endingDialogue.size());
        for (std::size_t index = 0; index < dialogueBeforeDoor; ++index)
        {
            if (!showDialogue(tutorial_.endingDialogue.at(index))) return false;
        }
        for (auto step = 0; step < 30 && window_.isOpen(); ++step)
        {
            doorOffset_ = static_cast<float>(step) * 12.0F;
            drawFrame();
            window_.display();
            std::this_thread::sleep_for(std::chrono::milliseconds{20});
        }
        for (std::size_t index = dialogueBeforeDoor; index < tutorial_.endingDialogue.size(); ++index)
        {
            const auto& line = tutorial_.endingDialogue.at(index);
            if (!showDialogue(line)) return false;
            if (line.text.starts_with("Ein Sofa")) animatePlayerEntrance(3U);
            if (line.text.starts_with("Ein Hase")) animatePlayerEntrance(2U);
            if (line.text.starts_with("Ein Computer")) animatePlayerEntrance(1U);
            if (line.text.starts_with("Eine Werkbank")) animatePlayerEntrance(0U);
        }
        return true;
    }

    bool InitialTutorialSession::showDialogue(const content::TutorialDialogueLine& line)
    {
        auto speaker = std::size_t{};
        if (line.speaker == "Ole") speaker = 1U;
        else if (line.speaker == "Anna") speaker = 2U;
        else if (line.speaker == "Markus") speaker = 3U;

        sf::Text bubbleText{font_, "", 25U};
        bubbleText.setFillColor(sf::Color::Black);
        const auto source = line.speaker + ": " + line.text;
        std::istringstream words{source};
        std::string word;
        std::string currentLine;
        std::string wrapped;
        while (words >> word)
        {
            const auto candidate = currentLine.empty() ? word : currentLine + " " + word;
            bubbleText.setString(sf::String::fromUtf8(candidate.begin(), candidate.end()));
            if (!currentLine.empty() && bubbleText.getLocalBounds().size.x > 390.0F)
            {
                wrapped += (wrapped.empty() ? "" : "\n") + currentLine;
                currentLine = word;
            }
            else
            {
                currentLine = candidate;
            }
        }
        wrapped += (wrapped.empty() ? "" : "\n") + currentLine;
        bubbleText.setString(sf::String::fromUtf8(wrapped.begin(), wrapped.end()));
        const auto textBounds = bubbleText.getLocalBounds();
        NinePatch bubble{speechBubbleTexture_};
        bubble.setSize({textBounds.size.x + 70.0F, textBounds.size.y + 60.0F});

        while (window_.isOpen())
        {
            while (const auto event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window_.close();
                    return false;
                }
                if (const auto* key = event->getIf<sf::Event::KeyPressed>();
                    key && (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space))
                    return true;
                if (const auto* click = event->getIf<sf::Event::MouseButtonReleased>(); click && click->button == sf::Mouse::Button::Left)
                    return true;
            }
            drawFrame();
            window_.setView(worldView_);
            const auto bounds = party_.at(speaker)->bounds();
            const auto bubblePosition = sf::Vector2f{bounds.left + bounds.width - 20.0F, bounds.top - bubble.size().y};
            bubble.setPosition(bubblePosition);
            bubbleText.setPosition(bubblePosition + sf::Vector2f{35.0F, 25.0F} - textBounds.position);
            window_.draw(bubble);
            window_.draw(bubbleText);
            window_.setView(hudView_);
            window_.display();
        }
        return false;
    }

    void InitialTutorialSession::animatePlayerEntrance(std::size_t playerIndex)
    {
        constexpr auto steps = 45;
        const auto& partyPosition = world_.presentation.partyPositions.at(playerIndex);
        const auto start = sf::Vector2f{viewLeft_ + static_cast<float>(partyPosition.x), static_cast<float>(partyPosition.y)};
        const auto destination = sf::Vector2f{roomWidth * 2.0F + static_cast<float>(tutorial_.doorPosition.x) + 150.0F,
                                             static_cast<float>(tutorial_.doorPosition.y + 360)};
        party_.at(playerIndex)->play(world_.presentation.walkAnimation, static_cast<float>(world_.presentation.walkAnimationSpeed));
        for (auto step = 1; step <= steps && window_.isOpen(); ++step)
        {
            const auto progress = static_cast<float>(step) / static_cast<float>(steps);
            const auto position = start + (destination - start) * progress;
            party_.at(playerIndex)->setPosition(position.x, position.y);
            party_.at(playerIndex)->update(20.0F);
            drawFrame();
            window_.display();
            std::this_thread::sleep_for(std::chrono::milliseconds{20});
        }
        entered_.at(playerIndex) = true;
    }
} // namespace soma::presentation
