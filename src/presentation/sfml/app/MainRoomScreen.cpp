#include "presentation/sfml/app/MainRoomScreen.hpp"

#include "presentation/sfml/common/AudioSettings.hpp"

#include "game/CampaignProgression.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation
{
    namespace
    {
        sf::Vector2f vector(const content::Point& point)
        {
            return {static_cast<float>(point.x), static_cast<float>(point.y)};
        }

        sf::Sprite positioned(const sf::Texture& texture, const content::Point& point)
        {
            sf::Sprite sprite{texture};
            sprite.setPosition(vector(point));
            return sprite;
        }

        sf::Text centeredNumber(const sf::Font& font, int value, sf::Vector2f center, unsigned int size)
        {
            sf::Text text{font, std::to_string(value), size};
            text.setFillColor(sf::Color::White);
            text.setOutlineColor(sf::Color::Black);
            text.setOutlineThickness(2.0F);
            const auto bounds = text.getLocalBounds();
            text.setPosition(center - bounds.position - bounds.size / 2.0F);
            return text;
        }
    }

    MainRoomScreen::MainRoomScreen(sf::RenderWindow& window,
                                   const content::GameFlowContent& flow,
                                   const content::WorldContent& world,
                                   const persistence::SaveGame& saveGame)
        : window_(window)
        , flow_(flow)
        , world_(world)
        , saveGame_(saveGame)
        , worldView_(sf::FloatRect{{static_cast<float>(flow.mainRoom.initialViewLeft), 0.0F},
                                   {static_cast<float>(flow.referenceSize.x), static_cast<float>(flow.referenceSize.y)}})
        , hudView_(sf::FloatRect{{0.0F, 0.0F}, {static_cast<float>(flow.referenceSize.x), static_cast<float>(flow.referenceSize.y)}})
        , rootsTexture_(flow.mainRoom.roots.texture)
        , vendingTexture_(flow.mainRoom.vendingMachine.texture)
        , statusBarBackgroundTexture_(flow.mainRoom.statusBarBackground)
        , diceIconTexture_(flow.mainRoom.diceIcon)
        , cardsIconTexture_(flow.mainRoom.cardsIcon)
        , resourceFont_(flow.mainRoom.resourceFont)
        , normalCursorImage_(flow.mainRoom.normalCursor)
        , handCursorImage_(flow.mainRoom.handCursor)
        , normalCursor_(normalCursorImage_.getPixelsPtr(), normalCursorImage_.getSize(), {0U, 0U})
        , handCursor_(handCursorImage_.getPixelsPtr(), handCursorImage_.getSize(), {0U, 0U})
        , music_(flow.mainRoom.music)
        , pauseMenu_(flow, PauseMenuContext::mainRoom)
    {
        for (std::size_t index = 0; index < backgroundTextures_.size(); ++index)
        {
            backgroundTextures_.at(index) = sf::Texture{flow.mainRoom.backgrounds.at(index)};
        }
        for (std::size_t index = 0; index < doorTextures_.size(); ++index)
        {
            doorTextures_.at(index) = sf::Texture{flow.mainRoom.doors.at(index).texture};
            doorSoundBuffers_.at(index) = sf::SoundBuffer{flow.mainRoom.doors.at(index).sound};
            signTextures_.at(index) = sf::Texture{flow.mainRoom.signs.at(index).texture};
            signFonts_.at(index) = sf::Font{flow.mainRoom.signs.at(index).font};
        }
        for (std::size_t index = 0; index < toolbarTextures_.size(); ++index)
        {
            toolbarTextures_.at(index) = sf::Texture{flow.mainRoom.toolbarButtons.at(index)};
        }
        for (std::size_t index = 0; index < players_.size(); ++index)
        {
            const auto& player = flow.mainRoom.players.at(index);
            players_.at(index) = std::make_unique<SpriterAnimationPlayer>(window_, player.model, player.entity);
            players_.at(index)->setPosition(static_cast<float>(player.position.x), static_cast<float>(player.position.y));
            players_.at(index)->play(player.idleAnimation, 0.5F);
            scheduleAmbientAnimation(index);
        }
        music_->setLooping(true);
        music_->setRelativeToListener(true);
        music_->setVolume(AudioSettings::music(static_cast<float>(flow.mainRoom.musicVolume)));
        music_->play();
        outlineAvailable_ = outlineShader_.loadFromFile(flow.mainRoom.outlineShader, sf::Shader::Type::Fragment);
        if (outlineAvailable_)
        {
            outlineShader_.setUniform("u_texture", sf::Shader::CurrentTexture);
            outlineShader_.setUniform("u_outlineColor", sf::Glsl::Vec4{1.0F, 0.72F, 0.18F, 1.0F});
            outlineShader_.setUniform("u_textureRect", sf::Glsl::Vec4{0.0F, 0.0F, 1.0F, 1.0F});
        }
        window_.setMouseCursor(normalCursor_);
    }

    MainRoomResult MainRoomScreen::run()
    {
        while (window_.isOpen() && !result_)
        {
            processEvents();
            update(clock_.restart());
            updateCursor();
            render();
        }
        return result_.value_or(MainRoomResult{});
    }

    void MainRoomScreen::resume()
    {
        result_.reset();
        static_cast<void>(clock_.restart());
        if (music_->getStatus() != sf::SoundSource::Status::Playing)
        {
            music_->play();
        }
    }

    void MainRoomScreen::renderBackdrop()
    {
        window_.setView(worldView_);
        drawWorld();
        window_.setView(hudView_);
        drawHud();
    }

    void MainRoomScreen::stopMusic()
    {
        music_->stop();
    }

    void MainRoomScreen::processEvents()
    {
        while (const auto event = window_.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window_.close();
                result_ = MainRoomResult{MainRoomAction::windowClosed};
                continue;
            }
            const auto pauseAction = pauseMenu_.handleEvent(*event, window_);
            if (pauseAction == PauseMenuAction::saveAndMainMenu)
            {
                result_ = MainRoomResult{MainRoomAction::mainMenu};
                continue;
            }
            if (pauseAction || pauseMenu_.isOpen())
            {
                continue;
            }
            if (const auto* click = event->getIf<sf::Event::MouseButtonReleased>();
                click && click->button == sf::Mouse::Button::Left)
            {
                handleClick(window_.mapPixelToCoords(click->position, worldView_), window_.mapPixelToCoords(click->position, hudView_));
            }
        }
    }

    void MainRoomScreen::handleClick(sf::Vector2f worldPosition, sf::Vector2f hudPosition)
    {
        for (std::size_t index = 0; index < players_.size(); ++index)
        {
            const auto bounds = players_.at(index)->bounds();
            if (sf::FloatRect{{bounds.left, bounds.top}, {bounds.width, bounds.height}}.contains(worldPosition))
            {
                result_ = MainRoomResult{MainRoomAction::skills, {}, index};
                return;
            }
        }
        sf::Sprite vending{vendingTexture_};
        vending.setPosition(vector(flow_.mainRoom.vendingMachine.position));
        if (vending.getGlobalBounds().contains(worldPosition))
        {
            result_ = MainRoomResult{MainRoomAction::shop};
            return;
        }
        for (std::size_t index = 0; index < flow_.mainRoom.doors.size(); ++index)
        {
            const auto& door = flow_.mainRoom.doors.at(index);
            sf::Sprite sprite{doorTextures_.at(index)};
            sprite.setPosition(vector(door.position));
            if (door.enabled && campaignAvailable(door.campaign) && sprite.getGlobalBounds().contains(worldPosition))
            {
                doorSound_ = std::make_unique<sf::Sound>(doorSoundBuffers_.at(index));
                doorSound_->setVolume(AudioSettings::sound());
                doorSound_->play();
                result_ = MainRoomResult{MainRoomAction::enterCampaign, door.campaign};
                return;
            }
        }
        if (const auto action = toolbarActionAt(hudPosition))
        {
            interfaceSounds_.click();
            result_ = MainRoomResult{*action};
        }
    }

    std::optional<MainRoomAction> MainRoomScreen::toolbarActionAt(sf::Vector2f position) const
    {
        static constexpr std::array actions{MainRoomAction::inventory,
                                            MainRoomAction::skills,
                                            MainRoomAction::shop,
                                            MainRoomAction::rules};
        for (std::size_t index = 0; index < toolbarTextures_.size(); ++index)
        {
            const auto buttonPosition = vector(flow_.mainRoom.toolbarPositions.at(index));
            const sf::FloatRect bounds{buttonPosition,
                                       {static_cast<float>(toolbarTextures_.at(index).getSize().x / 2U),
                                        static_cast<float>(toolbarTextures_.at(index).getSize().y)}};
            if (bounds.contains(position))
            {
                return actions.at(index);
            }
        }
        return std::nullopt;
    }

    void MainRoomScreen::update(sf::Time elapsed)
    {
        music_->setVolume(AudioSettings::music(static_cast<float>(flow_.mainRoom.musicVolume)));
        if (pauseMenu_.isOpen())
        {
            return;
        }
        updatePlayerAnimations(elapsed);
        const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_);
        auto direction = 0.0F;
        if (mouse.x < static_cast<float>(flow_.mainRoom.edgeScrollWidth))
        {
            direction = -(1.0F - mouse.x / static_cast<float>(flow_.mainRoom.edgeScrollWidth));
        }
        else if (mouse.x > static_cast<float>(flow_.referenceSize.x - flow_.mainRoom.edgeScrollWidth))
        {
            direction = (mouse.x - static_cast<float>(flow_.referenceSize.x - flow_.mainRoom.edgeScrollWidth)) /
                        static_cast<float>(flow_.mainRoom.edgeScrollWidth);
        }
        const auto roomWidth = static_cast<float>(backgroundTextures_.front().getSize().x * backgroundTextures_.size());
        const auto maximumLeft = roomWidth - static_cast<float>(flow_.referenceSize.x);
        const auto left = std::clamp(worldView_.getCenter().x - worldView_.getSize().x / 2.0F +
                                         direction * static_cast<float>(flow_.mainRoom.edgeScrollSpeed) * elapsed.asSeconds(),
                                     0.0F,
                                     maximumLeft);
        worldView_.setCenter({left + worldView_.getSize().x / 2.0F, worldView_.getCenter().y});
        sf::Listener::setPosition({worldView_.getCenter().x, worldView_.getCenter().y, -5.0F});
    }

    void MainRoomScreen::updatePlayerAnimations(sf::Time elapsed)
    {
        for (std::size_t index = 0; index < players_.size(); ++index)
        {
            auto& player = players_.at(index);
            player->update(static_cast<float>(elapsed.asMilliseconds()));
            if (playingAmbientAnimation_.at(index))
            {
                if (!player->isPlaying())
                {
                    player->play(flow_.mainRoom.players.at(index).idleAnimation, 0.5F);
                    playingAmbientAnimation_.at(index) = false;
                    scheduleAmbientAnimation(index);
                }
                continue;
            }
            ambientAnimationDelays_.at(index) -= elapsed.asSeconds();
            const auto& animations = flow_.mainRoom.players.at(index).ambientAnimations;
            if (ambientAnimationDelays_.at(index) <= 0.0F && !animations.empty())
            {
                std::uniform_int_distribution<std::size_t> animation(0, animations.size() - 1);
                player->play(animations.at(animation(random_)), 0.5F);
                playingAmbientAnimation_.at(index) = true;
            }
            else if (!player->isPlaying())
            {
                player->play(flow_.mainRoom.players.at(index).idleAnimation, 0.5F);
            }
        }
    }

    void MainRoomScreen::scheduleAmbientAnimation(std::size_t playerIndex)
    {
        const auto& definition = flow_.mainRoom.players.at(playerIndex);
        std::uniform_real_distribution<float> delay(static_cast<float>(definition.minimumAmbientDelay),
                                                    static_cast<float>(definition.maximumAmbientDelay));
        ambientAnimationDelays_.at(playerIndex) = delay(random_);
    }

    std::string MainRoomScreen::levelSignLabel(std::string_view campaign) const
    {
        const game::CampaignProgression progression{world_, saveGame_.completedLevels};
        if (progression.levels(campaign).empty())
        {
            return "-";
        }
        const auto next = progression.nextLevel(campaign);
        return next ? std::to_string(world_.levels.at(*next).difficulty) : "✓";
    }

    bool MainRoomScreen::campaignAvailable(std::string_view campaign) const
    {
        return game::CampaignProgression{world_, saveGame_.completedLevels}.nextLevel(campaign).has_value();
    }

    bool MainRoomScreen::interactiveAt(sf::Vector2f position) const
    {
        for (const auto& player : players_)
        {
            const auto bounds = player->bounds();
            if (sf::FloatRect{{bounds.left, bounds.top}, {bounds.width, bounds.height}}.contains(position)) return true;
        }
        sf::Sprite vending{vendingTexture_};
        vending.setPosition(vector(flow_.mainRoom.vendingMachine.position));
        if (vending.getGlobalBounds().contains(position)) return true;
        for (std::size_t index = 0; index < flow_.mainRoom.doors.size(); ++index)
        {
            sf::Sprite door{doorTextures_.at(index)};
            door.setPosition(vector(flow_.mainRoom.doors.at(index).position));
            const auto& definition = flow_.mainRoom.doors.at(index);
            if (definition.enabled && campaignAvailable(definition.campaign) && door.getGlobalBounds().contains(position)) return true;
        }
        return false;
    }

    void MainRoomScreen::updateCursor()
    {
        const auto pixelPosition = sf::Mouse::getPosition(window_);
        const auto hand = interactiveAt(window_.mapPixelToCoords(pixelPosition, worldView_)) ||
                          toolbarActionAt(window_.mapPixelToCoords(pixelPosition, hudView_)).has_value();
        if (hand != handCursorActive_)
        {
            handCursorActive_ = hand;
            window_.setMouseCursor(hand ? handCursor_ : normalCursor_);
        }
    }

    void MainRoomScreen::drawWorld()
    {
        const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), worldView_);
        for (std::size_t index = 0; index < backgroundTextures_.size(); ++index)
        {
            sf::Sprite background{backgroundTextures_.at(index)};
            background.setPosition({static_cast<float>(index * backgroundTextures_.at(index).getSize().x), 0.0F});
            window_.draw(background);
        }
        for (std::size_t index = 0; index < doorTextures_.size(); ++index)
        {
            const auto& door = flow_.mainRoom.doors.at(index);
            const auto sprite = positioned(doorTextures_.at(index), door.position);
            drawOutlinedSprite(doorTextures_.at(index), door.position,
                               door.enabled && campaignAvailable(door.campaign) && sprite.getGlobalBounds().contains(mouse));
        }
        window_.draw(positioned(rootsTexture_, flow_.mainRoom.roots.position));
        for (std::size_t index = 0; index < signTextures_.size(); ++index)
        {
            const auto& sign = flow_.mainRoom.signs.at(index);
            auto sprite = positioned(signTextures_.at(index), sign.position);
            window_.draw(sprite);
            const auto campaign = flow_.mainRoom.doors.at(index).campaign;
            const auto center = sprite.getGlobalBounds().getCenter() + vector(sign.textOffset);
            const auto value = levelSignLabel(campaign);
            sf::Text text{signFonts_.at(index), sf::String::fromUtf8(value.begin(), value.end()), 70U};
            text.setFillColor(sf::Color::White);
            text.setOutlineColor(sf::Color::Black);
            text.setOutlineThickness(2.0F);
            const auto bounds = text.getLocalBounds();
            text.setPosition(center - bounds.position - bounds.size / 2.0F);
            window_.draw(text);
        }
        for (auto& player : players_)
        {
            const auto bounds = player->bounds();
            drawPlayer(*player, sf::FloatRect{{bounds.left, bounds.top}, {bounds.width, bounds.height}}.contains(mouse));
        }
        const auto vending = positioned(vendingTexture_, flow_.mainRoom.vendingMachine.position);
        drawOutlinedSprite(vendingTexture_, flow_.mainRoom.vendingMachine.position, vending.getGlobalBounds().contains(mouse));
    }

    void MainRoomScreen::drawOutlinedSprite(const sf::Texture& texture, const content::Point& position, bool outlined)
    {
        const auto sprite = positioned(texture, position);
        if (outlined && outlineAvailable_)
        {
            outlineShader_.setUniform("u_textureSize",
                                      sf::Glsl::Vec2{static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y)});
            constexpr float tau = 6.28318530718F;
            for (auto sample = 0; sample < flow_.mainRoom.outlineSamples; ++sample)
            {
                const auto angle = tau * static_cast<float>(sample) / static_cast<float>(flow_.mainRoom.outlineSamples);
                auto outline = sprite;
                outline.move({std::cos(angle) * static_cast<float>(flow_.mainRoom.outlineRadius),
                              std::sin(angle) * static_cast<float>(flow_.mainRoom.outlineRadius)});
                window_.draw(outline, &outlineShader_);
            }
        }
        window_.draw(sprite);
    }

    void MainRoomScreen::drawPlayer(SpriterAnimationPlayer& player, bool outlined)
    {
        if (outlined)
        {
            constexpr float tau = 6.28318530718F;
            player.setTint(sf::Color{255, 184, 46});
            for (auto sample = 0; sample < flow_.mainRoom.outlineSamples; ++sample)
            {
                const auto angle = tau * static_cast<float>(sample) / static_cast<float>(flow_.mainRoom.outlineSamples);
                player.setRenderOffset({std::cos(angle) * static_cast<float>(flow_.mainRoom.outlineRadius),
                                        std::sin(angle) * static_cast<float>(flow_.mainRoom.outlineRadius)});
                player.renderVisual();
            }
            player.setTint(sf::Color::White);
            player.setRenderOffset({});
        }
        player.render();
    }

    void MainRoomScreen::drawHud()
    {
        const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_);
        const auto pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        window_.draw(positioned(statusBarBackgroundTexture_, flow_.mainRoom.statusBarBackgroundPosition));
        for (std::size_t index = 0; index < toolbarTextures_.size(); ++index)
        {
            sf::Sprite button{toolbarTextures_.at(index)};
            button.setPosition(vector(flow_.mainRoom.toolbarPositions.at(index)));
            const auto frameWidth = static_cast<int>(toolbarTextures_.at(index).getSize().x / 2U);
            const sf::FloatRect bounds{button.getPosition(),
                                       {static_cast<float>(frameWidth), static_cast<float>(toolbarTextures_.at(index).getSize().y)}};
            const auto frame = pressed && bounds.contains(mouse) ? 1 : 0;
            button.setTextureRect({{frame * frameWidth, 0},
                                   {frameWidth, static_cast<int>(toolbarTextures_.at(index).getSize().y)}});
            window_.draw(button);
        }
        auto dice = positioned(diceIconTexture_, flow_.mainRoom.diceIconPosition);
        auto cards = positioned(cardsIconTexture_, flow_.mainRoom.cardsIconPosition);
        const auto diceScale = static_cast<float>(flow_.mainRoom.diceIconScale);
        const auto cardsScale = static_cast<float>(flow_.mainRoom.cardsIconScale);
        dice.setScale({diceScale, diceScale});
        cards.setScale({cardsScale, cardsScale});
        window_.draw(dice);
        window_.draw(cards);
        window_.draw(centeredNumber(resourceFont_, saveGame_.campaign.currency(gameplay::Currency::dice),
                                    vector(flow_.mainRoom.diceTextCenter), static_cast<unsigned int>(flow_.mainRoom.resourceTextSize)));
        window_.draw(centeredNumber(resourceFont_, saveGame_.campaign.currency(gameplay::Currency::cards),
                                    vector(flow_.mainRoom.cardsTextCenter), static_cast<unsigned int>(flow_.mainRoom.resourceTextSize)));
    }

    void MainRoomScreen::render()
    {
        window_.clear(sf::Color::Black);
        window_.setView(worldView_);
        drawWorld();
        window_.setView(hudView_);
        drawHud();
        pauseMenu_.render(window_, window_.mapPixelToCoords(sf::Mouse::getPosition(window_), hudView_));
        window_.display();
    }
} // namespace soma::presentation
