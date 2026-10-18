#include "content/EconomyContentReader.hpp"

#include <stdexcept>

namespace soma::content::detail
{
    void loadPresentation(const Json& document, EconomyContent& content, const std::filesystem::path& contentRoot)
    {
        const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
        for (const auto& [idValue, pathValue] : document.at("assets").items())
        {
            const gameplay::AssetId id{idValue};
            const std::filesystem::path path{pathValue.get<std::string>()};
            if (!std::filesystem::exists(repositoryRoot / path))
            {
                throw std::runtime_error("Missing economy presentation asset: " + path.string());
            }
            if (!content.assets.emplace(id, path).second)
            {
                throw std::runtime_error("Duplicate economy asset ID: " + id.value());
            }
        }

        auto& presentation = content.presentation;
        presentation.referenceSize = readPoint(document.at("reference_size"));
        const auto& resources = document.at("resource_counter");
        presentation.resourceCounter = {readPoint(resources.at("dice_icon")),
                                        readPoint(resources.at("dice_text_center")),
                                        readPoint(resources.at("cards_icon")),
                                        readPoint(resources.at("cards_text_center")),
                                        resources.at("dice_scale").get<double>(),
                                        resources.at("cards_scale").get<double>(),
                                        integer(resources, "text_size")};
        const auto& inventory = document.at("inventory");
        presentation.inventory = {readPoint(inventory.at("origin")),
                                  readPoint(inventory.at("items_panel")),
                                  readPoint(inventory.at("equipment_panel")),
                                  readPoint(inventory.at("attributes")),
                                  readPoint(inventory.at("previous_button")),
                                  readPoint(inventory.at("next_button")),
                                  readPoint(inventory.at("close_button")),
                                  readArray<4>(inventory.at("equipment_fields"), readPoint),
                                  readPoint(inventory.at("equipment_item_inset")),
                                  readPoint(inventory.at("equipment_background")),
                                  readPoint(inventory.at("equipment_diamond")),
                                  readArray<4>(inventory.at("equipment_connections"), readPoint),
                                  readRectangle(inventory.at("items_scrollbar"))};

        const auto& shop = document.at("shop");
        presentation.shop = {readPoint(shop.at("vending_origin")),
                             readPoint(shop.at("inventory_panel")),
                             readPoint(shop.at("consumables_row")),
                             readPoint(shop.at("shop_panel")),
                             readPoint(shop.at("shop_item_origin")),
                             readPoint(shop.at("shop_item_spacing")),
                             readPoint(shop.at("consumable_origin")),
                             readPoint(shop.at("consumable_spacing")),
                             readPoint(shop.at("dice")),
                             readPoint(shop.at("dice_price_lock")),
                             readPoint(shop.at("buy_button")),
                             readPoint(shop.at("close_button")),
                             readPoint(shop.at("title")),
                             readPoint(shop.at("offer_title")),
                             readPoint(shop.at("selection_inset")),
                             readPoint(shop.at("price_sign_offset")),
                             readPoint(shop.at("slot_lock_offset")),
                             readRectangle(shop.at("slot_unlock_price")),
                             readPoint(shop.at("reduced_price_sign_offset")),
                             readRectangle(shop.at("dice_unlock_price")),
                             readPoint(shop.at("action_price")),
                             readPoint(shop.at("action_cards")),
                             readPoint(shop.at("confirmation_message")),
                             integer(shop, "confirmation_message_width"),
                             readPoint(shop.at("confirmation_yes_button")),
                             readPoint(shop.at("confirmation_no_button")),
                             readPoint(shop.at("sell_slider")),
                             readPoint(shop.at("sell_confirm_button")),
                             readPoint(shop.at("sell_cancel_button")),
                             readRectangle(shop.at("sell_quantity_bounds")),
                             readPoint(shop.at("owned_item_inset")),
                             readPoint(shop.at("owned_item_spacing")),
                             readPoint(shop.at("item_size")),
                             readPoint(shop.at("quantity_offset"))};

        const auto& skills = document.at("skills");
        presentation.skills = {readPoint(skills.at("origin")),
                               readRectangle(skills.at("player_name_bounds")),
                               readRectangle(skills.at("panel_title_bounds")),
                               readArray<4>(skills.at("ability_hitboxes"), readRectangle),
                               readRectangle(skills.at("ability_name_bounds")),
                               readRectangle(skills.at("ability_icon_bounds")),
                               readArray<4>(skills.at("ability_bridges"), readPoint),
                               readArray<2>(skills.at("tree_origins"), readPoint),
                               readPoint(skills.at("version_ability_icon")),
                               readArray<2>(skills.at("version_selectors"), readPoint),
                               readArray<5>(skills.at("skill_icons"), readPoint),
                               readArray<4>(skills.at("connection_offsets"), readPoint),
                               readArray<4>(skills.at("connection_types"),
                                            [](const Json& value) {
                                                return value.get<int>();
                                            }),
                               readPoint(skills.at("acquired_inset")),
                               readPoint(skills.at("selection_inset")),
                               readPoint(skills.at("dice")),
                               readPoint(skills.at("buy_button")),
                               readPoint(skills.at("previous_button")),
                               readPoint(skills.at("next_button")),
                               readPoint(skills.at("close_button")),
                               readRectangle(skills.at("selected_skill_title_bounds")),
                               readPoint(skills.at("dice_cost_offset"))};
        const auto& drawer = document.at("consumable_drawer");
        presentation.consumableDrawer = {readPoint(drawer.at("open_position")),
                                         integer(drawer, "closed_x"),
                                         readPoint(drawer.at("toggle_size")),
                                         readPoint(drawer.at("toggle_inset")),
                                         readPoint(drawer.at("row_inset")),
                                         readPoint(drawer.at("first_item_inset")),
                                         integer(drawer, "item_spacing")};
        const auto& explorationConsumables = document.at("exploration_consumables");
        presentation.explorationConsumables = {readPoint(explorationConsumables.at("position")),
                                               readPoint(explorationConsumables.at("row_inset")),
                                               readPoint(explorationConsumables.at("first_item_inset")),
                                               readPoint(explorationConsumables.at("quantity_offset")),
                                               integer(explorationConsumables, "item_spacing"),
                                               integer(explorationConsumables, "item_size")};
        if (presentation.referenceSize.x <= 0 || presentation.referenceSize.y <= 0)
        {
            throw std::runtime_error("Economy presentation reference size must be positive");
        }
    }
} // namespace soma::content::detail
