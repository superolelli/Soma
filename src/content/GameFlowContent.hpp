#pragma once

#include "content/EconomyContent.hpp"

#include <array>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace soma::content
{
    struct GameFlowContent
    {
        Point referenceSize;
        std::unordered_map<std::string, std::filesystem::path> assets;
        struct Startup
        {
            std::filesystem::path model;
            std::string entity;
            std::string animation;
            Point position;
            double speed{1.0};
            double minimumDisplaySeconds{};
        } startup;
        struct PauseMenu
        {
            Point panelPosition;
            Point panelSize;
            Point titleCenter;
            Point firstButton;
            Point musicSlider;
            Point soundSlider;
            Point battleSpeedSlider;
            Point settingsBackButton;
            int sliderWidth{};
            int buttonSpacing{};
            int titleSize{};
            int textSize{};
        } pauseMenu;
        struct Menu
        {
            std::array<Point, 3> savePanelPositions;
            Point closeButtonPosition;
            int firstActionOffsetY{};
            int actionSpacingY{};
            Point nameInputPosition;
            Point nameTextCenter;
            Point nameButtonPosition;
            Point feedbackTextCenter;
            int panelTextSize{};
            int buttonTextSize{};
            int nameTextSize{};
            int feedbackTextSize{};
            int maximumNameCharacters{};
        } menu;
        struct MainRoom
        {
            struct Door
            {
                std::string campaign;
                std::filesystem::path texture;
                Point position;
                std::filesystem::path sound;
                bool enabled{true};
            };
            struct Sign
            {
                std::filesystem::path texture;
                Point position;
                std::filesystem::path font;
                Point textOffset;
            };
            struct PositionedAsset
            {
                std::filesystem::path texture;
                Point position;
            };
            struct Player
            {
                std::filesystem::path model;
                std::string entity;
                Point position;
                std::string idleAnimation{"idle"};
                std::vector<std::string> ambientAnimations;
                double minimumAmbientDelay{8.0};
                double maximumAmbientDelay{18.0};
            };

            std::array<std::filesystem::path, 4> backgrounds;
            std::array<Door, 3> doors;
            std::array<Sign, 3> signs;
            PositionedAsset roots;
            PositionedAsset vendingMachine;
            std::array<Player, 4> players;
            std::array<std::filesystem::path, 4> toolbarButtons;
            std::filesystem::path diceIcon;
            std::filesystem::path cardsIcon;
            std::filesystem::path resourceFont;
            std::filesystem::path outlineShader;
            std::filesystem::path normalCursor;
            std::filesystem::path handCursor;
            std::filesystem::path music;
            int musicVolume{};
            int edgeScrollWidth{};
            int edgeScrollSpeed{};
            int initialViewLeft{};
            std::array<Point, 4> toolbarPositions;
            Point diceIconPosition;
            Point diceTextCenter;
            Point cardsIconPosition;
            Point cardsTextCenter;
            double diceIconScale{1.0};
            double cardsIconScale{1.0};
            int resourceTextSize{};
            int outlineSamples{};
            double outlineRadius{};
        } mainRoom;
    };
} // namespace soma::content
