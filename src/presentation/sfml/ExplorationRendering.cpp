#include "presentation/sfml/ExplorationSessionController.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation::detail
{
    void ExplorationSession::update(sf::Time elapsed)
    {
        const auto direction = movementDirection();
        const auto viewLeft =
            std::clamp(view_.getCenter().x - view_.getSize().x / 2.0F + direction * static_cast<float>(layout_.movementSpeed) * elapsed.asSeconds(),
                       0.0F,
                       maximumViewLeft_);
        view_.setCenter({viewLeft + view_.getSize().x / 2.0F, view_.getCenter().y});
        window_.setView(view_);
        particles_.update(elapsed, {{viewLeft, view_.getCenter().y - view_.getSize().y / 2.0F}, view_.getSize()});
        enterVisibleRoom(viewLeft);
        updateParty(elapsed, direction, viewLeft);
        updateSpeechBubbles(elapsed);
        updatePartyStatus(elapsed);
    }

    void ExplorationSession::updatePartyStatus(sf::Time elapsed)
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

    void ExplorationSession::updateSpeechBubbles(sf::Time elapsed)
    {
        const auto bounds = partyBounds();
        const auto available = livingPartyMembers();
        const auto active = !pendingEncounter_ && !lootDialog_ && !levelResultDialog_ && !defeated_;
        speechBubbles_->update(elapsed, bounds, available, active);
    }

    std::array<AnimationBounds, 4> ExplorationSession::partyBounds()
    {
        std::array<AnimationBounds, 4> result;
        for (std::size_t index = 0; index < party_.size(); ++index)
        {
            result.at(index) = party_.at(index)->bounds();
        }
        return result;
    }

    std::array<bool, 4> ExplorationSession::livingPartyMembers() const
    {
        std::array<bool, 4> result;
        for (std::size_t index = 0; index < result.size(); ++index)
        {
            result.at(index) = playerHealth_.at(playerDefinitions().at(index)) > 0;
        }
        return result;
    }

    int ExplorationSession::effectiveMaximumHealth(const gameplay::ContentDefinitionId& id) const
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

    float ExplorationSession::movementDirection() const
    {
        if (pendingEncounter_ || lootDialog_ || levelResultDialog_ || defeated_)
        {
            return 0.0F;
        }
        return (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ? 1.0F : 0.0F) -
               (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ? 1.0F : 0.0F);
    }

    void ExplorationSession::enterVisibleRoom(float viewLeft)
    {
        const auto roomIndex = std::min(levelRun_.rooms().size() - 1, static_cast<std::size_t>(viewLeft / roomWidth_));
        if (roomIndex != levelRun_.currentRoom() && levelRun_.enterRoom(roomIndex))
        {
            rememberLevelBeforePendingEncounter();
        }
    }

    void ExplorationSession::updateParty(sf::Time elapsed, float direction, float viewLeft)
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
            party_.at(index)->setPosition(viewLeft + layout_.partyPositions.at(index).x, static_cast<float>(layout_.partyPositions.at(index).y));
            party_.at(index)->update(static_cast<float>(elapsed.asMilliseconds()));
        }
    }

    void ExplorationSession::render()
    {
        drawExplorationFrame();
        window_.display();
        window_.setView(view_);
    }

    void ExplorationSession::drawExplorationFrame()
    {
        window_.setView(view_);
        window_.clear(sf::Color::Black);
        renderParallax();
        renderRooms();
        particles_.draw(window_, content::ParticleLayer::behindCharacters);
        renderParty();
        particles_.draw(window_, content::ParticleLayer::foreground);
        renderPartyStatus();
        speechBubbles_->render(window_);
        window_.setView(hudView_);
        renderHud();
    }

    void ExplorationSession::renderParallax()
    {
        const auto& parallax = levelRun_.parallax();
        const std::array fourthLayer{parallax.fourthLayer};
        renderParallaxLayer(fourthLayer, parallax.fourthLayerDivisor);
        renderParallaxLayer(parallax.thirdLayerTiles, parallax.thirdLayerDivisor);
        renderParallaxLayer(parallax.secondLayerTiles, parallax.secondLayerDivisor);
    }

    void ExplorationSession::renderParallaxLayer(std::span<const gameplay::AssetId> tiles, int scrollDivisor)
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

    void ExplorationSession::renderRooms()
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
                lootable.setPosition({static_cast<float>(index) * roomWidth_ + definition.position.x, static_cast<float>(definition.position.y)});
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

    void ExplorationSession::renderParty()
    {
        for (std::size_t index = 0; index < party_.size(); ++index)
        {
            if (playerHealth_.at(playerDefinitions().at(index)) > 0)
            {
                party_.at(index)->render();
            }
        }
    }

    void ExplorationSession::renderPartyStatus()
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

    void ExplorationSession::renderHud()
    {
        explorationHud_->render(window_, campaignState_.currency(gameplay::Currency::cards), campaignState_.currency(gameplay::Currency::dice));
        const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_);
        if (!lootDialog_ && !levelResultDialog_ && !pendingEncounter_)
        {
            explorationConsumables_->render(window_, mouse);
        }
        activeRules_->render(window_, mouse);
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

} // namespace soma::presentation::detail
