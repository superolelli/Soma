#pragma once

#include "content/Geometry.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace soma::content
{
    struct TutorialMessage
    {
        std::string id;
        std::string title;
        std::string text;
        std::vector<Rectangle> highlights;
        std::string button{"Weiter"};
    };

    struct TutorialDialogueLine
    {
        std::string speaker;
        std::string text;
    };

    struct TutorialOnboardingStep
    {
        std::string message;
        std::string action;
        std::string requirement;
    };

    struct TutorialContent
    {
        std::filesystem::path introVideo;
        std::filesystem::path firstBackground;
        std::filesystem::path battleBackground;
        std::filesystem::path endBackground;
        std::filesystem::path door;
        std::filesystem::path font;
        std::string lootable;
        std::string rewardItem;
        int rewardCards{};
        int rewardDice{};
        Point doorPosition;
        int movementSpeed{380};
        std::unordered_map<std::string, std::string> initialMessages;
        std::vector<std::string> battleMessages;
        std::vector<TutorialDialogueLine> endingDialogue;
        std::vector<TutorialOnboardingStep> mainRoomOnboarding;
        std::unordered_map<std::string, std::string> onDemandMessages;
        std::unordered_map<std::string, TutorialMessage> messages;
    };
} // namespace soma::content
