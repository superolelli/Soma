#include "content/flow/GameFlowContentLoader.hpp"

#include <fstream>
#include <json.hpp>
#include <stdexcept>

namespace soma::content
{
    namespace
    {
        Point point(const nlohmann::json& value)
        {
            if (!value.is_array() || value.size() != 2)
            {
                throw std::runtime_error("Expected a two-component point");
            }
            return {value.at(0).get<int>(), value.at(1).get<int>()};
        }
    }

    bool GameFlowContentLoadResult::valid() const noexcept
    {
        return diagnostics.empty();
    }

    GameFlowContentLoadResult GameFlowContentLoader::load(const std::filesystem::path& contentRoot) const
    {
        GameFlowContentLoadResult result;
        const auto path = contentRoot / "flow/presentation.json";
        try
        {
            std::ifstream stream{path};
            if (!stream)
            {
                throw std::runtime_error("file could not be opened");
            }
            nlohmann::json document;
            stream >> document;
            if (document.at("schema_version").get<int>() != 1 ||
                document.at("content_kind").get<std::string>() != "game_flow_presentation")
            {
                throw std::runtime_error("unsupported schema or content kind");
            }
            result.content.referenceSize = point(document.at("reference_size"));
            const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
            for (const auto& [id, asset] : document.at("assets").items())
            {
                const auto assetPath = std::filesystem::path{asset.get<std::string>()};
                if (!std::filesystem::exists(repositoryRoot / assetPath))
                {
                    throw std::runtime_error("missing asset: " + assetPath.string());
                }
                result.content.assets.emplace(id, assetPath);
            }
            const auto& startup = document.at("startup");
            result.content.startup.model = startup.at("model").get<std::string>();
            if (!std::filesystem::exists(repositoryRoot / result.content.startup.model))
            {
                throw std::runtime_error("missing startup model: " + result.content.startup.model.string());
            }
            result.content.startup.entity = startup.at("entity").get<std::string>();
            result.content.startup.animation = startup.at("animation").get<std::string>();
            result.content.startup.position = point(startup.at("position"));
            result.content.startup.speed = startup.at("speed").get<double>();
            result.content.startup.minimumDisplaySeconds = startup.at("minimum_display_seconds").get<double>();
            const auto& pauseMenu = document.at("pause_menu");
            result.content.pauseMenu = {point(pauseMenu.at("panel_position")),
                                        point(pauseMenu.at("panel_size")),
                                        point(pauseMenu.at("title_center")),
                                        point(pauseMenu.at("first_button")),
                                        point(pauseMenu.at("music_slider")),
                                        point(pauseMenu.at("sound_slider")),
                                        point(pauseMenu.at("battle_speed_slider")),
                                        point(pauseMenu.at("settings_back_button")),
                                        pauseMenu.at("slider_width").get<int>(),
                                        pauseMenu.at("button_spacing").get<int>(),
                                        pauseMenu.at("title_size").get<int>(),
                                        pauseMenu.at("text_size").get<int>()};
            const auto& menu = document.at("menu");
            for (std::size_t index = 0; index < result.content.menu.savePanelPositions.size(); ++index)
            {
                result.content.menu.savePanelPositions.at(index) = point(menu.at("save_panel_positions").at(index));
            }
            result.content.menu.closeButtonPosition = point(menu.at("close_button_position"));
            result.content.menu.firstActionOffsetY = menu.at("first_action_offset_y").get<int>();
            result.content.menu.actionSpacingY = menu.at("action_spacing_y").get<int>();
            result.content.menu.nameInputPosition = point(menu.at("name_input_position"));
            result.content.menu.nameTextCenter = point(menu.at("name_text_center"));
            result.content.menu.nameButtonPosition = point(menu.at("name_button_position"));
            result.content.menu.feedbackTextCenter = point(menu.at("feedback_text_center"));
            result.content.menu.panelTextSize = menu.at("panel_text_size").get<int>();
            result.content.menu.buttonTextSize = menu.at("button_text_size").get<int>();
            result.content.menu.nameTextSize = menu.at("name_text_size").get<int>();
            result.content.menu.feedbackTextSize = menu.at("feedback_text_size").get<int>();
            result.content.menu.maximumNameCharacters = menu.at("maximum_name_characters").get<int>();

            const auto& room = document.at("main_room");
            const auto checkedPath = [&](const nlohmann::json& value) {
                const auto resultPath = std::filesystem::path{value.get<std::string>()};
                if (!std::filesystem::exists(repositoryRoot / resultPath))
                {
                    throw std::runtime_error("missing main-room asset: " + resultPath.string());
                }
                return resultPath;
            };
            for (std::size_t index = 0; index < result.content.mainRoom.backgrounds.size(); ++index)
            {
                result.content.mainRoom.backgrounds.at(index) = checkedPath(room.at("backgrounds").at(index));
            }
            for (std::size_t index = 0; index < result.content.mainRoom.doors.size(); ++index)
            {
                const auto& source = room.at("doors").at(index);
                auto& destination = result.content.mainRoom.doors.at(index);
                destination.campaign = source.at("campaign").get<std::string>();
                destination.texture = checkedPath(source.at("texture"));
                destination.position = point(source.at("position"));
                destination.sound = checkedPath(source.at("sound"));
                destination.enabled = source.value("enabled", true);
            }
            for (std::size_t index = 0; index < result.content.mainRoom.signs.size(); ++index)
            {
                const auto& source = room.at("signs").at(index);
                auto& destination = result.content.mainRoom.signs.at(index);
                destination.texture = checkedPath(source.at("texture"));
                destination.position = point(source.at("position"));
                destination.font = checkedPath(source.at("font"));
                destination.textOffset = point(source.at("text_offset"));
            }
            const auto positionedAsset = [&](std::string_view name) {
                const auto& source = room.at(std::string{name});
                return GameFlowContent::MainRoom::PositionedAsset{checkedPath(source.at("texture")), point(source.at("position"))};
            };
            result.content.mainRoom.roots = positionedAsset("roots");
            result.content.mainRoom.vendingMachine = positionedAsset("vending_machine");
            for (std::size_t index = 0; index < result.content.mainRoom.players.size(); ++index)
            {
                const auto& source = room.at("players").at(index);
                auto& player = result.content.mainRoom.players.at(index);
                player.model = checkedPath(source.at("model"));
                player.entity = source.at("entity").get<std::string>();
                player.position = point(source.at("position"));
                player.idleAnimation = source.value("idle_animation", std::string{"idle"});
                player.ambientAnimations = source.value("ambient_animations", std::vector<std::string>{});
                player.minimumAmbientDelay = source.value("minimum_ambient_delay", 8.0);
                player.maximumAmbientDelay = source.value("maximum_ambient_delay", 18.0);
                if (player.minimumAmbientDelay <= 0.0 || player.maximumAmbientDelay < player.minimumAmbientDelay)
                    throw std::runtime_error("Invalid main-room ambient animation interval");
            }
            result.content.mainRoom.toolbarButtons = {checkedPath(room.at("inventory_button")),
                                                       checkedPath(room.at("skills_button")),
                                                       checkedPath(room.at("shop_button")),
                                                       checkedPath(room.at("rules_button"))};
            result.content.mainRoom.statusBarBackground = checkedPath(room.at("status_bar_background"));
            result.content.mainRoom.diceIcon = checkedPath(room.at("dice_icon"));
            result.content.mainRoom.cardsIcon = checkedPath(room.at("cards_icon"));
            result.content.mainRoom.resourceFont = checkedPath(room.at("resource_font"));
            result.content.mainRoom.outlineShader = checkedPath(room.at("outline_shader"));
            result.content.mainRoom.normalCursor = checkedPath(room.at("cursor_normal"));
            result.content.mainRoom.handCursor = checkedPath(room.at("cursor_hand"));
            result.content.mainRoom.music = checkedPath(room.at("music"));
            result.content.mainRoom.musicVolume = room.at("music_volume").get<int>();
            result.content.mainRoom.edgeScrollWidth = room.at("edge_scroll_width").get<int>();
            result.content.mainRoom.edgeScrollSpeed = room.at("edge_scroll_speed").get<int>();
            result.content.mainRoom.initialViewLeft = room.at("initial_view_left").get<int>();
            for (std::size_t index = 0; index < result.content.mainRoom.toolbarPositions.size(); ++index)
            {
                result.content.mainRoom.toolbarPositions.at(index) = point(room.at("toolbar_positions").at(index));
            }
            result.content.mainRoom.statusBarBackgroundPosition = point(room.at("status_bar_background_position"));
            result.content.mainRoom.diceIconPosition = point(room.at("dice_icon_position"));
            result.content.mainRoom.diceTextCenter = point(room.at("dice_text_center"));
            result.content.mainRoom.cardsIconPosition = point(room.at("cards_icon_position"));
            result.content.mainRoom.cardsTextCenter = point(room.at("cards_text_center"));
            result.content.mainRoom.diceIconScale = room.at("dice_icon_scale").get<double>();
            result.content.mainRoom.cardsIconScale = room.at("cards_icon_scale").get<double>();
            result.content.mainRoom.resourceTextSize = room.at("resource_text_size").get<int>();
            result.content.mainRoom.outlineSamples = room.at("outline_samples").get<int>();
            result.content.mainRoom.outlineRadius = room.at("outline_radius").get<double>();
        }
        catch (const std::exception& error)
        {
            result.diagnostics.push_back(
                {gameplay::DiagnosticSeverity::error, "flow.load", path, {}, error.what()});
        }
        return result;
    }
} // namespace soma::content
