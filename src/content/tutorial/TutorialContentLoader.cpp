#include "content/tutorial/TutorialContentLoader.hpp"

#include <fstream>
#include <algorithm>
#include <json.hpp>
#include <stdexcept>

namespace soma::content
{
    namespace
    {
        using json = nlohmann::json;

        Point point(const json& value)
        {
            return {value.at(0).get<int>(), value.at(1).get<int>()};
        }

        Rectangle rectangle(const json& value)
        {
            return {value.at(0).get<int>(), value.at(1).get<int>(), value.at(2).get<int>(), value.at(3).get<int>()};
        }

        std::filesystem::path requiredPath(const json& value, const std::filesystem::path& repositoryRoot)
        {
            auto path = std::filesystem::path{value.get<std::string>()};
            if (path.is_relative()) path = repositoryRoot / path;
            if (!std::filesystem::exists(path))
            {
                throw std::runtime_error("Tutorial asset does not exist: " + path.string());
            }
            return path;
        }
    } // namespace

    bool TutorialContentLoadResult::valid() const
    {
        return std::ranges::none_of(diagnostics, [](const auto& diagnostic) {
            return diagnostic.severity == gameplay::DiagnosticSeverity::error;
        });
    }

    TutorialContentLoadResult TutorialContentLoader::load(const std::filesystem::path& contentRoot) const
    {
        TutorialContentLoadResult result;
        const auto path = contentRoot / "tutorial" / "content.json";
        const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
        try
        {
            std::ifstream input{path};
            if (!input)
            {
                throw std::runtime_error("Could not open tutorial content");
            }
            const auto document = json::parse(input);
            if (document.at("schema_version").get<int>() != 1)
            {
                throw std::runtime_error("Unsupported tutorial schema version");
            }

            auto& content = result.content;
            content.introVideo = requiredPath(document.at("intro_video"), repositoryRoot);
            const auto& level = document.at("initial_level");
            content.firstBackground = requiredPath(level.at("first_background"), repositoryRoot);
            content.battleBackground = requiredPath(level.at("battle_background"), repositoryRoot);
            content.endBackground = requiredPath(level.at("end_background"), repositoryRoot);
            content.door = requiredPath(level.at("door"), repositoryRoot);
            content.font = requiredPath(document.at("font"), repositoryRoot);
            content.lootable = level.at("lootable").get<std::string>();
            content.rewardItem = level.at("loot_item").get<std::string>();
            content.rewardCards = level.at("battle_reward").at("cards").get<int>();
            content.rewardDice = level.at("battle_reward").at("dice").get<int>();
            content.doorPosition = point(level.at("door_position"));
            content.movementSpeed = level.value("movement_speed", 380);
            for (const auto& [trigger, message] : document.at("initial_messages").items())
            {
                content.initialMessages.emplace(trigger, message.get<std::string>());
            }
            content.battleMessages = document.at("battle_messages").get<std::vector<std::string>>();

            for (const auto& line : document.at("ending_dialogue"))
            {
                content.endingDialogue.push_back({line.at("speaker").get<std::string>(), line.at("text").get<std::string>()});
            }
            for (const auto& step : document.at("main_room_onboarding"))
            {
                content.mainRoomOnboarding.push_back({step.at("message").get<std::string>(),
                                                      step.at("action").get<std::string>(),
                                                      step.value("requirement", std::string{})});
            }
            for (const auto& [trigger, message] : document.at("on_demand_messages").items())
            {
                content.onDemandMessages.emplace(trigger, message.get<std::string>());
            }
            for (const auto& value : document.at("messages"))
            {
                TutorialMessage message{value.at("id").get<std::string>(),
                                        value.at("title").get<std::string>(),
                                        value.at("text").get<std::string>(),
                                        {},
                                        value.value("button", "Weiter")};
                if (value.contains("highlights"))
                {
                    for (const auto& highlight : value.at("highlights"))
                    {
                        message.highlights.push_back(rectangle(highlight));
                    }
                }
                else if (value.contains("highlight"))
                {
                    message.highlights.push_back(rectangle(value.at("highlight")));
                }
                if (!content.messages.emplace(message.id, std::move(message)).second)
                {
                    throw std::runtime_error("Duplicate tutorial message id");
                }
            }

            const auto requireMessage = [&content](const std::string& id) {
                if (!content.messages.contains(id))
                {
                    throw std::runtime_error("Tutorial references unknown message: " + id);
                }
            };
            for (const auto trigger : {"navigation", "lootable", "battle_loot", "retry_battle"})
            {
                if (!content.initialMessages.contains(trigger))
                {
                    throw std::runtime_error("Tutorial is missing initial message trigger: " + std::string{trigger});
                }
            }
            for (const auto& [trigger, id] : content.initialMessages)
            {
                static_cast<void>(trigger);
                requireMessage(id);
            }
            for (const auto& id : content.battleMessages) requireMessage(id);
            for (const auto& step : content.mainRoomOnboarding) requireMessage(step.message);
            for (const auto& [trigger, id] : content.onDemandMessages)
            {
                static_cast<void>(trigger);
                requireMessage(id);
            }
        }
        catch (const std::exception& error)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "tutorial.load", path, {}, error.what()});
        }
        return result;
    }
} // namespace soma::content
