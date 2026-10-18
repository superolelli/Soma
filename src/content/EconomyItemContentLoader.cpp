#include "content/EconomyContentReader.hpp"
#include "gameplay/items/ItemRarity.hpp"

#include <stdexcept>

namespace soma::content::detail
{
    gameplay::AttributeBonuses readBonuses(const Json& value)
    {
        gameplay::AttributeBonuses result;
        result.armour = integer(value, "armour");
        result.maximumHealth = integer(value, "maxHealth");
        result.damageMinimum = integer(value, "damageMin");
        result.damageMaximum = integer(value, "damageMax");
        result.initiative = integer(value, "initiative");
        result.criticalHitChance = integer(value, "criticalHit");
        result.dodge = integer(value, "dodge");
        result.precision = integer(value, "precision");
        result.healing = integer(value, "healing");
        result.sleepResistance = integer(value, "sleepResistance");
        result.confusionResistance = integer(value, "confusionResistance");
        result.debuffResistance = integer(value, "debuffResistance");
        result.decayResistance = integer(value, "decayResistance");
        return result;
    }

    gameplay::ItemColor readItemColor(const Json& value)
    {
        if (!value.is_array() || value.size() != 3)
        {
            throw std::runtime_error("Expected a three-component item color");
        }
        for (const auto& component : value)
        {
            if (!component.is_number_integer() || component.get<int>() < 0 || component.get<int>() > 255)
            {
                throw std::runtime_error("Item color components must be between 0 and 255");
            }
        }
        return {static_cast<std::uint8_t>(value.at(0).get<int>()),
                static_cast<std::uint8_t>(value.at(1).get<int>()),
                static_cast<std::uint8_t>(value.at(2).get<int>())};
    }

    gameplay::ItemCategory readCategory(const std::string& value)
    {
        if (value == "currency")
        {
            return gameplay::ItemCategory::currency;
        }
        if (value == "equipment")
        {
            return gameplay::ItemCategory::equipment;
        }
        if (value == "consumable")
        {
            return gameplay::ItemCategory::consumable;
        }
        throw std::runtime_error("Unknown item category: " + value);
    }

    std::optional<gameplay::EquipmentBattleEffect> readEquipmentBattleEffect(const Json& value)
    {
        if (!value.contains("battle_effect"))
        {
            return std::nullopt;
        }
        const auto& effect = value.at("battle_effect");
        const auto type = effect.at("type").get<std::string>();
        if (type == "lone_victory")
        {
            const auto threshold = effect.at("other_party_health_threshold").get<int>();
            const auto damage = effect.at("damage").get<int>();
            if (threshold < 0 || damage <= 0)
            {
                throw std::runtime_error("Invalid lone-victory equipment effect");
            }
            return gameplay::LoneVictoryEffect{threshold, damage};
        }
        throw std::runtime_error("Unknown equipment battle effect: " + type);
    }

    gameplay::ItemProperties readProperties(const Json& value, gameplay::ItemCategory category)
    {
        if (category == gameplay::ItemCategory::currency)
        {
            const auto currency = value.at("currency").get<std::string>();
            if (currency == "cards")
            {
                return gameplay::CurrencyItem{gameplay::Currency::cards};
            }
            if (currency == "dice")
            {
                return gameplay::CurrencyItem{gameplay::Currency::dice};
            }
            throw std::runtime_error("Unknown currency: " + currency);
        }
        if (category == gameplay::ItemCategory::equipment)
        {
            return gameplay::EquipmentItem{readBonuses(value.at("bonuses")),
                                           value.value("miss_on_high_damage", false),
                                           value.value("heal_on_pass", false),
                                           readEquipmentBattleEffect(value)};
        }
        return gameplay::ConsumableItem{integer(value, "healing"), readBonuses(value.at("bonuses")), integer(value, "duration")};
    }

    void loadItems(const Json& document, EconomyContent& content, const std::filesystem::path& contentRoot)
    {
        const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
        for (const auto& value : document.at("diamond_bonuses"))
        {
            std::optional<gameplay::ItemColor> requiredConnectionColor;
            if (value.contains("requires_connection_color"))
            {
                requiredConnectionColor = readItemColor(value.at("requires_connection_color"));
            }
            content.diamondBonuses.push_back({readItemColor(value.at("color")), requiredConnectionColor, readBonuses(value.at("bonuses"))});
        }
        for (const auto& value : document.at("items"))
        {
            const gameplay::ItemId id{value.at("id").get<std::string>()};
            const gameplay::AssetId icon{id.value() + ".icon"};
            const auto iconPath = std::filesystem::path{value.at("icon").get<std::string>()};
            const auto category = readCategory(value.at("category").get<std::string>());
            gameplay::ItemDefinition definition{id,
                                                value.at("name").get<std::string>(),
                                                category,
                                                integer(value, "level", 1),
                                                integer(value, "price"),
                                                readProperties(value, category),
                                                icon,
                                                value.at("campaigns").get<std::vector<std::string>>(),
                                                value.value("description", std::string{})};
            if (definition.name.empty() || definition.level < gameplay::minimumItemRarityLevel ||
                definition.level > gameplay::maximumItemRarityLevel || definition.price < 0 || definition.campaigns.empty())
            {
                throw std::runtime_error("Invalid item values: " + id.value());
            }
            for (const auto& campaign : definition.campaigns)
            {
                if (campaign != "bang" && campaign != "kutschfahrt")
                {
                    throw std::runtime_error("Unknown campaign '" + campaign + "' on item " + id.value());
                }
            }
            if (!std::filesystem::exists(repositoryRoot / iconPath))
            {
                throw std::runtime_error("Missing item icon: " + iconPath.string());
            }
            if (!content.items.emplace(id, std::move(definition)).second)
            {
                throw std::runtime_error("Duplicate item ID: " + id.value());
            }
            content.assets.emplace(icon, iconPath);
        }
    }

} // namespace soma::content::detail
