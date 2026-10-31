#pragma once

#include "content/flow/GameFlowContent.hpp"
#include "content/world/WorldContent.hpp"
#include "persistence/SaveGame.hpp"
#include "presentation/sfml/economy/EconomySoundPlayer.hpp"
#include "presentation/sfml/common/PauseMenu.hpp"
#include "presentation/sfml/common/SpriterAnimationPlayer.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <random>

namespace soma::presentation
{
    enum class MainRoomAction
    {
        none,
        mainMenu,
        inventory,
        skills,
        shop,
        rules,
        enterCampaign,
        windowClosed
    };

    struct MainRoomResult
    {
        MainRoomAction action{MainRoomAction::windowClosed};
        std::string campaign;
        std::optional<std::size_t> player;
    };

    /// Presents the persistent hub, campaign doors, ambient characters, toolbar, and pause menu.
    class MainRoomScreen
    {
      public:
        MainRoomScreen(sf::RenderWindow& window,
                       const content::GameFlowContent& flow,
                       const content::WorldContent& world,
                       const persistence::SaveGame& saveGame);

        [[nodiscard]] MainRoomResult run();
        void resume();
        void renderBackdrop();
        void stopMusic();
        [[nodiscard]] std::optional<MainRoomAction> toolbarActionAt(sf::Vector2f position) const;

      private:
        void processEvents();
        void handleClick(sf::Vector2f worldPosition, sf::Vector2f hudPosition);
        void update(sf::Time elapsed);
        void updatePlayerAnimations(sf::Time elapsed);
        void scheduleAmbientAnimation(std::size_t playerIndex);
        void render();
        void drawWorld();
        void drawHud();
        void drawOutlinedSprite(const sf::Texture& texture, const content::Point& position, bool outlined);
        void drawPlayer(SpriterAnimationPlayer& player, bool outlined);
        void updateCursor();
        [[nodiscard]] bool interactiveAt(sf::Vector2f position) const;
        [[nodiscard]] std::string levelSignLabel(std::string_view campaign) const;
        [[nodiscard]] bool campaignAvailable(std::string_view campaign) const;

        sf::RenderWindow& window_;
        const content::GameFlowContent& flow_;
        const content::WorldContent& world_;
        const persistence::SaveGame& saveGame_;
        sf::View worldView_;
        sf::View hudView_;
        std::array<sf::Texture, 4> backgroundTextures_;
        std::array<sf::Texture, 3> doorTextures_;
        std::array<sf::SoundBuffer, 3> doorSoundBuffers_;
        std::array<sf::Texture, 3> signTextures_;
        std::array<sf::Font, 3> signFonts_;
        sf::Texture rootsTexture_;
        sf::Texture vendingTexture_;
        std::array<sf::Texture, 4> toolbarTextures_;
        sf::Texture statusBarBackgroundTexture_;
        sf::Texture diceIconTexture_;
        sf::Texture cardsIconTexture_;
        sf::Font resourceFont_;
        sf::Shader outlineShader_;
        bool outlineAvailable_{};
        std::array<std::unique_ptr<SpriterAnimationPlayer>, 4> players_;
        std::array<float, 4> ambientAnimationDelays_{};
        std::array<bool, 4> playingAmbientAnimation_{};
        std::mt19937 random_{std::random_device{}()};
        sf::Image normalCursorImage_;
        sf::Image handCursorImage_;
        sf::Cursor normalCursor_;
        sf::Cursor handCursor_;
        std::optional<sf::Music> music_;
        std::unique_ptr<sf::Sound> doorSound_;
        EconomySoundPlayer interfaceSounds_;
        PauseMenu pauseMenu_;
        sf::Clock clock_;
        std::optional<MainRoomResult> result_;
        bool handCursorActive_{};
    };
} // namespace soma::presentation
