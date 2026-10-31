#include "persistence/SaveRepository.hpp"

#include "gameplay/combat/Fatigue.hpp"
#include "gameplay/rules/ComposedRule.hpp"

#include <algorithm>
#include <fstream>
#include <json.hpp>
#include <system_error>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace soma::persistence
{
    namespace
    {
        using json = nlohmann::json;

        std::string currencyName(gameplay::Currency currency)
        {
            return currency == gameplay::Currency::cards ? "cards" : "dice";
        }

        gameplay::ItemColor readColor(const json& value)
        {
            if (!value.is_array() || value.size() != 3)
            {
                throw std::runtime_error("An item color must contain exactly three channels");
            }
            const auto channel = [&value](std::size_t index) {
                const auto result = value.at(index).get<int>();
                if (result < 0 || result > 255)
                {
                    throw std::runtime_error("An item color channel is outside 0..255");
                }
                return static_cast<std::uint8_t>(result);
            };
            return {channel(0), channel(1), channel(2)};
        }

        gameplay::ItemInstance readItem(const json& value)
        {
            return {gameplay::ItemId{value.at("id").get<std::string>()}, gameplay::normalizedItemColor(readColor(value.at("color")))};
        }

        json writeItem(const gameplay::ItemInstance& item)
        {
            return {{"id", item.definition.value()}, {"color", {item.color.red, item.color.green, item.color.blue}}};
        }

        std::optional<gameplay::RuleFragmentId> readRulePart(const json& value, std::string_view name)
        {
            const auto key = std::string{name};
            if (!value.contains(key) || value.at(key).is_null()) return std::nullopt;
            return gameplay::RuleFragmentId{value.at(key).get<std::string>()};
        }

        gameplay::ComposedRule readRule(const json& value)
        {
            return {readRulePart(value, "trigger"), readRulePart(value, "subject"), readRulePart(value, "effect"),
                    readRulePart(value, "modifier")};
        }

        json writeRule(const gameplay::ComposedRule& rule)
        {
            json result = json::object();
            const auto write = [&result](std::string_view name, const std::optional<gameplay::RuleFragmentId>& part) {
                if (part) result[std::string{name}] = part->value();
            };
            write("trigger", rule.trigger);
            write("subject", rule.subject);
            write("effect", rule.effect);
            write("modifier", rule.modifier);
            return result;
        }

        game::ShopStockEntry readShopStockEntry(const json& value)
        {
            game::ShopStockEntry result;
            if (value.contains("item")) result.item = readItem(value.at("item"));
            if (value.contains("fragment")) result.fragment = gameplay::RuleFragmentId{value.at("fragment").get<std::string>()};
            return result;
        }

        json writeShopStockEntry(const game::ShopStockEntry& entry)
        {
            json result = json::object();
            if (entry.item) result["item"] = writeItem(*entry.item);
            if (entry.fragment) result["fragment"] = entry.fragment->value();
            return result;
        }

        game::CampaignStateData readCampaignData(const json& state)
        {
            game::CampaignStateData campaign;
            const auto& currencies = state.at("currencies");
            campaign.currencies[gameplay::Currency::cards] = currencies.at("cards").get<int>();
            campaign.currencies[gameplay::Currency::dice] = currencies.at("dice").get<int>();
            for (const auto& item : state.at("inventory")) campaign.inventory.push_back(readItem(item));
            for (const auto& item : state.at("consumables"))
                campaign.consumables.emplace(gameplay::ItemId{item.at("id").get<std::string>()}, item.at("quantity").get<int>());
            for (const auto& entry : state.at("equipment"))
                campaign.equipment.emplace(
                    game::EquipmentLocation{gameplay::ContentDefinitionId{entry.at("player").get<std::string>()},
                                            gameplay::EquipmentSlotId{entry.at("slot").get<std::string>()}},
                    readItem(entry.at("item")));
            for (const auto& item : state.at("unlocked_items")) campaign.unlockedItems.emplace(item.get<std::string>());
            for (const auto& skill : state.at("acquired_skills")) campaign.acquiredSkills.emplace(skill.get<std::string>());
            for (const auto& selection : state.at("active_ability_versions"))
            {
                campaign.activeAbilityVersions.emplace(
                    game::AbilitySetLocation{gameplay::ContentDefinitionId{selection.at("player").get<std::string>()},
                                             selection.at("slot").get<std::size_t>()},
                    selection.at("version").get<std::size_t>());
            }
            for (const auto& fragment : state.at("rule_fragments"))
                campaign.ruleFragments.emplace(gameplay::RuleFragmentId{fragment.at("id").get<std::string>()},
                                               fragment.at("quantity").get<int>());
            for (std::size_t index = 0; index < campaign.rules.size(); ++index)
                campaign.rules.at(index) = readRule(state.at("rules").at(index));
            if (state.contains("shop_stock"))
            {
                for (const auto& entry : state.at("shop_stock")) campaign.shopStock.push_back(readShopStockEntry(entry));
            }
            campaign.ruleSlotsUnlocked = state.at("rule_slots_unlocked").get<int>();
            campaign.shopSlotsUnlocked = state.at("shop_slots_unlocked").get<int>();
            campaign.shopDicePrice = state.at("shop_dice_price").get<int>();
            return campaign;
        }

        json writeCampaignData(const game::CampaignStateData& state)
        {
            json inventory = json::array();
            for (const auto& item : state.inventory) inventory.push_back(writeItem(item));
            json consumables = json::array();
            for (const auto& [id, quantity] : state.consumables)
                consumables.push_back({{"id", id.value()}, {"quantity", quantity}});
            json equipment = json::array();
            for (const auto& [location, item] : state.equipment)
                equipment.push_back({{"player", location.player.value()}, {"slot", location.slot.value()}, {"item", writeItem(item)}});
            json unlockedItems = json::array();
            for (const auto& item : state.unlockedItems) unlockedItems.push_back(item.value());
            json acquiredSkills = json::array();
            for (const auto& skill : state.acquiredSkills) acquiredSkills.push_back(skill.value());
            json activeAbilityVersions = json::array();
            for (const auto& [location, version] : state.activeAbilityVersions)
            {
                activeAbilityVersions.push_back(
                    {{"player", location.player.value()}, {"slot", location.slot}, {"version", version}});
            }
            json ruleFragments = json::array();
            for (const auto& [id, quantity] : state.ruleFragments)
                ruleFragments.push_back({{"id", id.value()}, {"quantity", quantity}});
            json rules = json::array();
            for (const auto& rule : state.rules) rules.push_back(writeRule(rule));
            json shopStock = json::array();
            for (const auto& entry : state.shopStock) shopStock.push_back(writeShopStockEntry(entry));
            return {{"currencies",
                     {{currencyName(gameplay::Currency::cards), state.currencies.at(gameplay::Currency::cards)},
                      {currencyName(gameplay::Currency::dice), state.currencies.at(gameplay::Currency::dice)}}},
                    {"inventory", std::move(inventory)},
                    {"consumables", std::move(consumables)},
                    {"equipment", std::move(equipment)},
                    {"unlocked_items", std::move(unlockedItems)},
                    {"acquired_skills", std::move(acquiredSkills)},
                    {"active_ability_versions", std::move(activeAbilityVersions)},
                    {"rule_fragments", std::move(ruleFragments)},
                    {"rules", std::move(rules)},
                    {"shop_stock", std::move(shopStock)},
                    {"rule_slots_unlocked", state.ruleSlotsUnlocked},
                    {"shop_slots_unlocked", state.shopSlotsUnlocked},
                    {"shop_dice_price", state.shopDicePrice}};
        }

        simulation::GeneratedRoom readRoom(const json& value)
        {
            simulation::GeneratedRoom room;
            room.background = gameplay::AssetId{value.at("background").get<std::string>()};
            if (value.contains("encounter")) room.encounter = gameplay::EncounterId{value.at("encounter").get<std::string>()};
            if (value.contains("lootable")) room.lootable = gameplay::LootableId{value.at("lootable").get<std::string>()};
            room.boss = value.at("boss").get<bool>();
            room.encounterPending = value.at("encounter_pending").get<bool>();
            room.lootableOpened = value.at("lootable_opened").get<bool>();
            return room;
        }

        json writeRoom(const simulation::GeneratedRoom& room)
        {
            json result{{"background", room.background.value()},
                        {"boss", room.boss},
                        {"encounter_pending", room.encounterPending},
                        {"lootable_opened", room.lootableOpened}};
            if (room.encounter) result["encounter"] = room.encounter->value();
            if (room.lootable) result["lootable"] = room.lootable->value();
            return result;
        }

        content::AttributeDelta readAttributeDelta(const json& value)
        {
            return {.armour = value.at("armour").get<int>(),
                    .damageMinimum = value.at("damage_minimum").get<int>(),
                    .damageMaximum = value.at("damage_maximum").get<int>(),
                    .initiative = value.at("initiative").get<int>(),
                    .maximumHealth = value.at("maximum_health").get<int>(),
                    .criticalHitChance = value.at("critical_hit_chance").get<int>(),
                    .precision = value.at("precision").get<int>(),
                    .dodge = value.at("dodge").get<int>(),
                    .healing = value.at("healing").get<int>(),
                    .sleepResistance = value.at("sleep_resistance").get<int>(),
                    .confusionResistance = value.at("confusion_resistance").get<int>(),
                    .debuffResistance = value.at("debuff_resistance").get<int>(),
                    .decayResistance = value.at("decay_resistance").get<int>()};
        }

        json writeAttributeDelta(const content::AttributeDelta& value)
        {
            return {{"armour", value.armour},
                    {"damage_minimum", value.damageMinimum},
                    {"damage_maximum", value.damageMaximum},
                    {"initiative", value.initiative},
                    {"maximum_health", value.maximumHealth},
                    {"critical_hit_chance", value.criticalHitChance},
                    {"precision", value.precision},
                    {"dodge", value.dodge},
                    {"healing", value.healing},
                    {"sleep_resistance", value.sleepResistance},
                    {"confusion_resistance", value.confusionResistance},
                    {"debuff_resistance", value.debuffResistance},
                    {"decay_resistance", value.decayResistance}};
        }

        simulation::ActiveBuff readActiveBuff(const json& value)
        {
            return {.attributes = readAttributeDelta(value.at("attributes")),
                    .remainingTurns = value.at("remaining_turns").get<int>(),
                    .positive = value.at("positive").get<bool>(),
                    .healingReceived = value.at("healing_received").get<double>(),
                    .damageAgainstMarked = value.at("damage_against_marked").get<double>(),
                    .damagePerNegativeStatus = value.at("damage_per_negative_status").get<double>(),
                    .negativeStatusCountAtApplication = value.at("negative_status_count_at_application").get<int>(),
                    .damageFromFatigue = value.at("damage_from_fatigue").get<bool>(),
                    .blocksFatigueStageOne = value.at("blocks_fatigue_stage_one").get<bool>(),
                    .blocksFatigueStageTwo = value.at("blocks_fatigue_stage_two").get<bool>()};
        }

        json writeActiveBuff(const simulation::ActiveBuff& value)
        {
            return {{"attributes", writeAttributeDelta(value.attributes)},
                    {"remaining_turns", value.remainingTurns},
                    {"positive", value.positive},
                    {"healing_received", value.healingReceived},
                    {"damage_against_marked", value.damageAgainstMarked},
                    {"damage_per_negative_status", value.damagePerNegativeStatus},
                    {"negative_status_count_at_application", value.negativeStatusCountAtApplication},
                    {"damage_from_fatigue", value.damageFromFatigue},
                    {"blocks_fatigue_stage_one", value.blocksFatigueStageOne},
                    {"blocks_fatigue_stage_two", value.blocksFatigueStageTwo}};
        }

        game::ActiveLevelState readActiveLevel(const json& value)
        {
            std::vector<simulation::GeneratedRoom> rooms;
            for (const auto& room : value.at("rooms")) rooms.push_back(readRoom(room));
            const auto& sourceParallax = value.at("parallax");
            simulation::GeneratedParallax parallax;
            for (const auto& tile : sourceParallax.at("second_layer_tiles"))
                parallax.secondLayerTiles.emplace_back(tile.get<std::string>());
            for (const auto& tile : sourceParallax.at("third_layer_tiles"))
                parallax.thirdLayerTiles.emplace_back(tile.get<std::string>());
            parallax.fourthLayer = gameplay::AssetId{sourceParallax.at("fourth_layer").get<std::string>()};
            parallax.secondLayerDivisor = sourceParallax.at("second_layer_divisor").get<int>();
            parallax.thirdLayerDivisor = sourceParallax.at("third_layer_divisor").get<int>();
            parallax.fourthLayerDivisor = sourceParallax.at("fourth_layer_divisor").get<int>();
            simulation::LevelRun run{gameplay::LevelId{value.at("level").get<std::string>()}, std::move(rooms), std::move(parallax)};
            if (!run.restoreProgress(value.at("current_room").get<std::size_t>()))
                throw std::runtime_error("Active level progress is invalid");
            game::ActiveLevelState result{std::move(run)};
            result.viewLeft = value.at("view_left").get<float>();
            for (const auto& [id, health] : value.at("player_health").items())
                result.progress.playerHealth.emplace(gameplay::ContentDefinitionId{id}, health.get<int>());
            for (const auto& [id, fatigue] : value.at("player_fatigue").items())
                result.progress.playerFatigue.emplace(gameplay::ContentDefinitionId{id}, fatigue.get<int>());
            for (const auto& [id, buffs] : value.at("player_buffs").items())
            {
                auto& destination = result.progress.playerBuffs[gameplay::ContentDefinitionId{id}];
                for (const auto& buff : buffs)
                {
                    destination.push_back(readActiveBuff(buff));
                }
            }
            for (const auto& ability : value.at("used_player_abilities"))
            {
                result.progress.usedPlayerAbilities.emplace(ability.get<std::string>());
            }
            result.progress.enemiesDefeated = value.at("enemies_defeated").get<int>();
            result.progress.defeated = value.at("defeated").get<bool>();
            result.baseline = readCampaignData(value.at("baseline"));
            return result;
        }

        json writeActiveLevel(const game::ActiveLevelState& active)
        {
            json rooms = json::array();
            for (const auto& room : active.run.rooms()) rooms.push_back(writeRoom(room));
            json second = json::array();
            for (const auto& tile : active.run.parallax().secondLayerTiles) second.push_back(tile.value());
            json third = json::array();
            for (const auto& tile : active.run.parallax().thirdLayerTiles) third.push_back(tile.value());
            json health = json::object();
            for (const auto& [id, value] : active.progress.playerHealth) health[id.value()] = value;
            json fatigue = json::object();
            for (const auto& [id, value] : active.progress.playerFatigue) fatigue[id.value()] = value;
            json buffs = json::object();
            for (const auto& [id, activeBuffs] : active.progress.playerBuffs)
            {
                auto& destination = buffs[id.value()] = json::array();
                for (const auto& buff : activeBuffs)
                {
                    destination.push_back(writeActiveBuff(buff));
                }
            }
            json usedAbilities = json::array();
            for (const auto& ability : active.progress.usedPlayerAbilities)
            {
                usedAbilities.push_back(ability.value());
            }
            return {{"level", active.run.level().value()},
                    {"rooms", std::move(rooms)},
                    {"current_room", active.run.currentRoom()},
                    {"view_left", active.viewLeft},
                    {"player_health", std::move(health)},
                    {"player_fatigue", std::move(fatigue)},
                    {"player_buffs", std::move(buffs)},
                    {"used_player_abilities", std::move(usedAbilities)},
                    {"enemies_defeated", active.progress.enemiesDefeated},
                    {"defeated", active.progress.defeated},
                    {"baseline", writeCampaignData(active.baseline)},
                    {"parallax",
                     {{"second_layer_tiles", std::move(second)},
                      {"third_layer_tiles", std::move(third)},
                      {"fourth_layer", active.run.parallax().fourthLayer.value()},
                      {"second_layer_divisor", active.run.parallax().secondLayerDivisor},
                      {"third_layer_divisor", active.run.parallax().thirdLayerDivisor},
                      {"fourth_layer_divisor", active.run.parallax().fourthLayerDivisor}}}};
        }

        SaveGame readSave(const json& document)
        {
            SaveGame result;
            result.schemaVersion = document.at("schema_version").get<int>();
            result.playerName = document.at("player_name").get<std::string>();
            result.campaign = game::CampaignState::fromData(readCampaignData(document.at("campaign")));
            for (const auto& level : document.at("completed_levels"))
            {
                result.completedLevels.emplace(level.get<std::string>());
            }
            if (document.contains("active_level") && !document.at("active_level").is_null())
                result.activeLevel = readActiveLevel(document.at("active_level"));
            return result;
        }

        json writeSave(const SaveGame& saveGame)
        {
            const auto state = saveGame.campaign.data();
            json completedLevels = json::array();
            for (const auto& level : saveGame.completedLevels)
            {
                completedLevels.push_back(level.value());
            }
            json result{{"schema_version", currentSaveVersion},
                    {"player_name", saveGame.playerName},
                    {"campaign", writeCampaignData(state)},
                    {"completed_levels", std::move(completedLevels)}};
            if (saveGame.activeLevel) result["active_level"] = writeActiveLevel(*saveGame.activeLevel);
            return result;
        }

        bool validEquipmentSlot(const gameplay::EquipmentSlotId& slot)
        {
            static constexpr std::array<std::string_view, 4> slots{
                "equipment.north", "equipment.east", "equipment.south", "equipment.west"};
            return std::ranges::find(slots, slot.value()) != slots.end();
        }

        std::optional<std::string> commitSaveFile(const std::filesystem::path& temporary,
                                                  const std::filesystem::path& destination)
        {
            std::error_code filesystemError;
            const auto destinationExists = std::filesystem::exists(destination, filesystemError);
            if (filesystemError)
            {
                return "Save destination could not be inspected: " + filesystemError.message();
            }

            if (!destinationExists)
            {
                if (MoveFileExW(temporary.c_str(), destination.c_str(), MOVEFILE_WRITE_THROUGH) != 0)
                {
                    return std::nullopt;
                }
                return "Save file could not be committed: " + std::system_category().message(static_cast<int>(GetLastError()));
            }

            const auto backup = std::filesystem::path{destination.string() + ".bak"};
            std::filesystem::remove(backup, filesystemError);
            if (filesystemError)
            {
                return "Previous save backup could not be removed: " + filesystemError.message();
            }
            if (ReplaceFileW(destination.c_str(), temporary.c_str(), backup.c_str(), REPLACEFILE_WRITE_THROUGH, nullptr, nullptr) == 0)
            {
                return "Save file could not be atomically replaced: " +
                       std::system_category().message(static_cast<int>(GetLastError()));
            }

            std::filesystem::remove(backup, filesystemError);
            return std::nullopt;
        }
    } // namespace

    SaveRepository::SaveRepository(const content::EconomyContent& economy,
                                   const content::WorldContent& world,
                                   const content::RuleContent& rules)
        : economy_(economy), world_(world), rules_(rules)
    {
    }

    gameplay::Result<SaveGame> SaveRepository::load(const std::filesystem::path& path) const
    {
        try
        {
            std::ifstream stream{path};
            if (!stream)
            {
                return gameplay::Result<SaveGame>::failure("Save file could not be opened");
            }
            json document;
            stream >> document;
            return validate(readSave(document));
        }
        catch (const std::exception& error)
        {
            return gameplay::Result<SaveGame>::failure(std::string{"Invalid save file: "} + error.what());
        }
    }

    gameplay::Result<std::filesystem::path> SaveRepository::save(const std::filesystem::path& path,
                                                                  const SaveGame& saveGame) const
    {
        try
        {
            const auto validated = validate(saveGame);
            if (!validated.hasValue())
            {
                return gameplay::Result<std::filesystem::path>::failure(validated.error());
            }
            std::filesystem::create_directories(path.parent_path());
            const auto temporary = std::filesystem::path{path.string() + ".tmp"};
            {
                std::ofstream stream{temporary, std::ios::trunc};
                if (!stream)
                {
                    return gameplay::Result<std::filesystem::path>::failure("Temporary save file could not be opened");
                }
                stream << writeSave(saveGame).dump(2) << '\n';
                if (!stream)
                {
                    return gameplay::Result<std::filesystem::path>::failure("Save file could not be written");
                }
                stream.close();
                if (!stream)
                {
                    return gameplay::Result<std::filesystem::path>::failure("Save file could not be finalized");
                }
            }
            if (const auto error = commitSaveFile(temporary, path))
            {
                return gameplay::Result<std::filesystem::path>::failure(*error);
            }
            return gameplay::Result<std::filesystem::path>::success(path);
        }
        catch (const std::exception& error)
        {
            return gameplay::Result<std::filesystem::path>::failure(std::string{"Save failed: "} + error.what());
        }
    }

    gameplay::Result<SaveGame> SaveRepository::validate(SaveGame saveGame) const
    {
        if (saveGame.schemaVersion != currentSaveVersion)
        {
            return gameplay::Result<SaveGame>::failure("Unsupported save schema version");
        }
        if (saveGame.playerName.empty() || saveGame.playerName.size() > 64)
        {
            return gameplay::Result<SaveGame>::failure("Player name must contain between 1 and 64 UTF-8 bytes");
        }
        const auto state = saveGame.campaign.data();
        if (!state.currencies.contains(gameplay::Currency::cards) || !state.currencies.contains(gameplay::Currency::dice))
        {
            return gameplay::Result<SaveGame>::failure("Save is missing a required currency value");
        }
        if (state.currencies.at(gameplay::Currency::cards) < 0 || state.currencies.at(gameplay::Currency::dice) < 0)
        {
            return gameplay::Result<SaveGame>::failure("Currency cannot be negative");
        }
        if (state.shopSlotsUnlocked < 1 || state.shopSlotsUnlocked > 5 || state.shopDicePrice < 50 ||
            state.shopDicePrice > 100 || state.shopDicePrice % 10 != 0)
        {
            return gameplay::Result<SaveGame>::failure("Shop progress is outside its supported range");
        }
        const auto validItem = [this](const gameplay::ItemId& id) { return economy_.items.contains(id); };
        if (std::ranges::any_of(state.inventory, [&](const auto& item) { return !validItem(item.definition); }) ||
            std::ranges::any_of(state.consumables, [&](const auto& entry) {
                const auto found = economy_.items.find(entry.first);
                return found == economy_.items.end() || found->second.category != gameplay::ItemCategory::consumable || entry.second <= 0;
            }) ||
            std::ranges::any_of(state.unlockedItems, [&](const auto& item) { return !validItem(item); }))
        {
            return gameplay::Result<SaveGame>::failure("Save references an invalid item");
        }
        if (state.shopStock.size() > 5U || std::ranges::any_of(state.shopStock, [this, &validItem](const auto& entry) {
                return (entry.item && entry.fragment) || (entry.item && !validItem(entry.item->definition)) ||
                       (entry.fragment && !rules_.fragments.contains(*entry.fragment));
            }))
        {
            return gameplay::Result<SaveGame>::failure("Save contains invalid shop stock");
        }
        for (const auto& [location, item] : state.equipment)
        {
            const auto found = economy_.items.find(item.definition);
            if (found == economy_.items.end() || found->second.category != gameplay::ItemCategory::equipment ||
                !validEquipmentSlot(location.slot))
            {
                return gameplay::Result<SaveGame>::failure("Save contains invalid equipment");
            }
        }
        if (std::ranges::any_of(state.acquiredSkills, [this](const auto& skill) { return !economy_.skills.contains(skill); }))
        {
            return gameplay::Result<SaveGame>::failure("Save references an invalid skill");
        }
        if (std::ranges::any_of(state.activeAbilityVersions, [this](const auto& selection) {
                const auto& [location, version] = selection;
                return version >= 2 || std::ranges::none_of(economy_.abilitySets, [&location](const auto& set) {
                           return set.player == location.player && set.slot == location.slot;
                       });
            }))
        {
            return gameplay::Result<SaveGame>::failure("Save references an invalid ability version");
        }
        if (state.ruleSlotsUnlocked < 1 || state.ruleSlotsUnlocked > static_cast<int>(gameplay::maximumRuleSlots) ||
            std::ranges::any_of(state.ruleFragments, [this](const auto& entry) {
                return !rules_.fragments.contains(entry.first) || entry.second <= 0;
            }))
        {
            return gameplay::Result<SaveGame>::failure("Rule progress is outside its supported range");
        }
        std::unordered_map<gameplay::RuleFragmentId, int> usedFragments;
        for (std::size_t slot = 0; slot < state.rules.size(); ++slot)
        {
            const auto& rule = state.rules.at(slot);
            if (slot >= static_cast<std::size_t>(state.ruleSlotsUnlocked) &&
                (rule.trigger || rule.subject || rule.effect || rule.modifier))
                return gameplay::Result<SaveGame>::failure("A locked rule slot contains fragments");
            const std::array parts{std::pair{&rule.trigger, content::RuleFragmentType::trigger},
                                   std::pair{&rule.subject, content::RuleFragmentType::subject},
                                   std::pair{&rule.effect, content::RuleFragmentType::effect},
                                   std::pair{&rule.modifier, content::RuleFragmentType::modifier}};
            bool triggered = true;
            bool standing = true;
            for (const auto& [part, expectedType] : parts)
            {
                if (!*part) continue;
                const auto found = rules_.fragments.find(**part);
                if (found == rules_.fragments.end() || found->second.type != expectedType)
                    return gameplay::Result<SaveGame>::failure("A rule references an invalid fragment");
                ++usedFragments[**part];
                triggered = triggered && found->second.allowedModes.supports(content::RuleMode::triggered);
                standing = standing && found->second.allowedModes.supports(content::RuleMode::standing);
            }
            if (!triggered && !standing) return gameplay::Result<SaveGame>::failure("A saved rule contains incompatible fragments");
        }
        for (const auto& [fragment, used] : usedFragments)
        {
            const auto owned = state.ruleFragments.find(fragment);
            if (owned == state.ruleFragments.end() || used > owned->second)
                return gameplay::Result<SaveGame>::failure("A saved rule uses more fragment copies than owned");
        }
        if (std::ranges::any_of(saveGame.completedLevels, [this](const auto& level) { return !world_.levels.contains(level); }))
        {
            return gameplay::Result<SaveGame>::failure("Save references an invalid level");
        }
        if (saveGame.activeLevel)
        {
            const auto& active = *saveGame.activeLevel;
            const auto& progress = active.progress;
            if (!world_.levels.contains(active.run.level()) || active.viewLeft < 0.0F || progress.defeated || progress.enemiesDefeated < 0 ||
                progress.playerHealth.empty() || progress.playerFatigue.size() != progress.playerHealth.size() ||
                std::ranges::any_of(progress.playerFatigue, [&active](const auto& entry) {
                    return !active.progress.playerHealth.contains(entry.first) || entry.second < 0 || entry.second > gameplay::maximumFatigue;
                }) ||
                std::ranges::any_of(progress.playerBuffs, [&progress](const auto& entry) {
                    return !progress.playerHealth.contains(entry.first) ||
                           std::ranges::any_of(entry.second, [](const simulation::ActiveBuff& buff) { return buff.remainingTurns <= 0; });
                }) ||
                std::ranges::any_of(active.run.rooms(), [this](const auto& room) {
                    return !world_.assets.contains(room.background) ||
                           (room.encounter && !world_.encounters.contains(*room.encounter)) ||
                           (room.lootable && !world_.lootables.contains(*room.lootable));
                }))
            {
                return gameplay::Result<SaveGame>::failure("Active level progress is invalid");
            }
        }
        return gameplay::Result<SaveGame>::success(std::move(saveGame));
    }
} // namespace soma::persistence
