#include "presentation/sfml/ExplorationSessionController.hpp"

#include <algorithm>

namespace soma::presentation::detail
{
    bool ExplorationSession::processEvents()
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
                const auto hudPosition = window_.mapPixelToCoords(click->position, hudView_);
                if (!lootDialog_ && !levelResultDialog_ && !pendingEncounter_ && activeRules_->contains(hudPosition))
                {
                    continue;
                }
                handleConsumableRelease(click->position);
                handleClick(click->position);
            }
            if (const auto* press = event->getIf<sf::Event::MouseButtonPressed>();
                press && press->button == sf::Mouse::Button::Left && !lootDialog_ && !levelResultDialog_ && !pendingEncounter_)
            {
                const auto hudPosition = window_.mapPixelToCoords(press->position, hudView_);
                if (activeRules_->handleClick(hudPosition))
                {
                    continue;
                }
                explorationConsumables_->press(hudPosition);
            }
        }
        return startBattle;
    }

    void ExplorationSession::handleConsumableRelease(sf::Vector2i pixel)
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
                const auto used =
                    explorationConsumableUse_.use(economy_.items.at(*item), playerHealth_.at(id), effectiveMaximumHealth(id), playerBuffs_[id]);
                if (used.hasValue())
                {
                    explorationConsumables_->playConsumedSound();
                }
                return;
            }
        }
    }

    bool ExplorationSession::handleKey(const sf::Event::KeyPressed& key)
    {
        if (key.code == sf::Keyboard::Key::Escape || (key.code == sf::Keyboard::Key::Enter && levelResultDialog_))
        {
            requestSessionClose();
            return false;
        }
        return key.code == sf::Keyboard::Key::Enter && pendingEncounter_.has_value();
    }

    void ExplorationSession::handleClick(sf::Vector2i pixel)
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

    void ExplorationSession::tryOpenLootable(sf::Vector2f mouse)
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

    void ExplorationSession::updateCursor()
    {
        const auto worldMouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), view_);
        const auto hudMouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_);
        const auto hand = isHoveringLootable(worldMouse) || (lootDialog_ && lootDialog_->clickableAt(hudMouse)) ||
                          (levelResultDialog_ && levelResultDialog_->clicked(hudMouse)) ||
                          (!lootDialog_ && !levelResultDialog_ && !pendingEncounter_ && activeRules_->clickableAt(hudMouse)) ||
                          (!lootDialog_ && !levelResultDialog_ && !pendingEncounter_ && explorationConsumables_->clickableAt(hudMouse));
        window_.setMouseCursor(hand ? *handCursor_ : *arrowCursor_);
    }

    bool ExplorationSession::isHoveringLootable(sf::Vector2f mouse) const
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

    sf::FloatRect ExplorationSession::lootableHitbox(std::size_t roomIndex) const
    {
        const auto& room = levelRun_.rooms().at(roomIndex);
        const auto& definition = world_.lootables.at(*room.lootable);
        const auto roomLeft = static_cast<float>(roomIndex) * roomWidth_;
        return {{roomLeft + definition.position.x + definition.hitbox.x, static_cast<float>(definition.position.y + definition.hitbox.y)},
                {static_cast<float>(definition.hitbox.width), static_cast<float>(definition.hitbox.height)}};
    }

    bool ExplorationSession::tryOpenLootable(std::size_t roomIndex, sf::Vector2f mouse)
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

} // namespace soma::presentation::detail
