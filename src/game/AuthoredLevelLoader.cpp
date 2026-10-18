#include "game/AuthoredLevelLoader.hpp"

#include <fstream>
#include <json.hpp>
#include <stdexcept>

namespace soma::game
{
    namespace
    {
        using json = nlohmann::json;

        template <typename Id, typename Definitions>
        std::optional<Id> optionalId(const json& object, std::string_view name, const Definitions& definitions)
        {
            const auto key = std::string{name};
            if (!object.contains(key) || object.at(key).is_null())
            {
                return std::nullopt;
            }
            const Id id{object.at(key).get<std::string>()};
            if (!definitions.contains(id))
            {
                throw std::runtime_error("Unknown " + std::string{name} + " in authored level: " + id.value());
            }
            return id;
        }

        std::vector<gameplay::AssetId> assetList(const json& values, const content::WorldContent& world)
        {
            std::vector<gameplay::AssetId> result;
            for (const auto& value : values)
            {
                gameplay::AssetId id{value.get<std::string>()};
                if (!world.assets.contains(id))
                {
                    throw std::runtime_error("Unknown asset in authored level: " + id.value());
                }
                result.push_back(std::move(id));
            }
            return result;
        }
    }

    simulation::LevelRun loadAuthoredLevel(const std::filesystem::path& path,
                                           const gameplay::LevelId& level,
                                           const content::WorldContent& world)
    {
        std::ifstream input{path};
        if (!input)
        {
            throw std::runtime_error("Could not open authored level: " + path.string());
        }
        const auto document = json::parse(input);
        std::vector<simulation::GeneratedRoom> rooms;
        for (const auto& value : document.at("rooms"))
        {
            gameplay::AssetId background{value.at("background").get<std::string>()};
            if (!world.assets.contains(background))
            {
                throw std::runtime_error("Unknown room background in authored level: " + background.value());
            }
            const auto encounter = optionalId<gameplay::EncounterId>(value, "encounter", world.encounters);
            rooms.push_back({std::move(background),
                             encounter,
                             optionalId<gameplay::LootableId>(value, "lootable", world.lootables),
                             value.value("boss", false),
                             encounter.has_value(),
                             false});
        }
        if (rooms.empty())
        {
            throw std::runtime_error("An authored level must contain at least one room");
        }

        const auto& source = document.at("parallax");
        gameplay::AssetId fourthLayer{source.at("fourth_layer").get<std::string>()};
        if (!world.assets.contains(fourthLayer))
        {
            throw std::runtime_error("Unknown fourth-layer asset in authored level: " + fourthLayer.value());
        }
        simulation::GeneratedParallax parallax{assetList(source.at("second_layer_tiles"), world),
                                                assetList(source.at("third_layer_tiles"), world),
                                                std::move(fourthLayer),
                                                source.value("second_layer_divisor", 3),
                                                source.value("third_layer_divisor", 6),
                                                source.value("fourth_layer_divisor", 9)};
        return simulation::LevelRun{level, std::move(rooms), std::move(parallax)};
    }
}
