#include "presentation/sfml/economy/InventoryPanel.hpp"

#include "game/EquipmentBonusCalculator.hpp"
#include "presentation/sfml/battle/BattleHud.hpp"
#include "presentation/sfml/economy/EconomyButton.hpp"
#include "presentation/sfml/economy/EconomyTooltip.hpp"
#include "presentation/sfml/economy/InventoryPanelDrawing.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation
{
    void InventoryPanel::drawFrame() const
    {
        const auto& layout = content_.presentation.inventory;
        sf::Sprite panel{textures_.at("inventory.panel")};
        panel.setPosition(absolute({0, 0}));
        window_.draw(panel);
        window_.draw(economyButton(textures_.at("skills.previous"), absolute(layout.previousButton), EconomyButtonBehavior::up,
                                   mousePosition_, mousePressed_));
        window_.draw(economyButton(textures_.at("skills.next"), absolute(layout.nextButton), EconomyButtonBehavior::up,
                                   mousePosition_, mousePressed_));
        window_.draw(economyButton(textures_.at("skills.close"), absolute(layout.closeButton), EconomyButtonBehavior::motionUp,
                                   mousePosition_, mousePressed_));
        window_.draw(detail::inventoryText(titleFont_, "Inventar", 70U, absolute({850, 70}), sf::Color::Black));
        auto player = detail::inventoryText(titleFont_, std::string{playerNames_.at(playerIndex_)}, 47U, {}, sf::Color::Black);
        player.setPosition(absolute({52 + static_cast<int>((162.0F - player.getLocalBounds().size.x) / 2.0F), 55}));
        window_.draw(player);
    }

    void InventoryPanel::drawEquipment() const
    {
        const auto summary = game::EquipmentBonusCalculator{content_}.calculate(state_, currentPlayer());
        drawEquipmentTopology(summary);
        drawEquippedItems();
        drawEquipmentStatistics(summary);
        drawDiamondTooltip(summary);
    }

    void InventoryPanel::drawEquipmentTopology(const game::EquipmentBonusSummary& summary) const
    {
        const auto& layout = content_.presentation.inventory;
        sf::Sprite background{textures_.at("inventory.equipment_background")};
        background.setPosition(equipmentOrigin() + sf::Vector2f{static_cast<float>(layout.equipmentBackground.x),
                                                                static_cast<float>(layout.equipmentBackground.y)});
        window_.draw(background);
        for (std::size_t connection = 0; connection < summary.connections.size(); ++connection)
        {
            const auto state = equipmentTransition_.connectionState(connection, summary);
            const auto offset = layout.equipmentConnections.at(connection);
            const auto position = equipmentOrigin() + sf::Vector2f{static_cast<float>(offset.x), static_cast<float>(offset.y)};
            connectionRenderer_.drawBase(window_, connection, position);
            if (state.has_value())
            {
                connectionRenderer_.draw(window_, connection, position, *state);
            }
        }
        sf::Sprite diamond{textures_.at("inventory.diamond")};
        diamond.setPosition(equipmentOrigin() + sf::Vector2f{static_cast<float>(layout.equipmentDiamond.x),
                                                             static_cast<float>(layout.equipmentDiamond.y)});
        diamond.setColor(equipmentTransition_.diamondColor(summary));
        window_.draw(diamond);
    }

    void InventoryPanel::drawEquippedItems() const
    {
        const auto& layout = content_.presentation.inventory;
        for (std::size_t slot = 0; slot < 4; ++slot)
        {
            sf::Sprite field{textures_.at("inventory.equipment_field")};
            field.setPosition(equipmentBounds(slot).position);
            window_.draw(field);
            const auto* equipped = state_.equipped(equipmentLocation(slot));
            if (equipped && (!dragged_ || dragged_->equipmentSlot != slot))
            {
                const auto& texture = textures_.at(content_.items.at(equipped->definition).icon);
                const auto itemPosition = equipmentBounds(slot).position +
                                          sf::Vector2f{static_cast<float>(layout.equipmentItemInset.x),
                                                       static_cast<float>(layout.equipmentItemInset.y)};
                sf::RectangleShape color{sf::Vector2f{texture.getSize()}};
                color.setPosition(itemPosition);
                const auto itemColor = gameplay::normalizedItemColor(equipped->color);
                color.setFillColor({itemColor.red, itemColor.green, itemColor.blue});
                window_.draw(color);
                sf::Sprite item{texture};
                item.setPosition(itemPosition);
                window_.draw(item);
            }
        }
    }

    void InventoryPanel::drawEquipmentStatistics(const game::EquipmentBonusSummary& summary) const
    {
        const auto& layout = content_.presentation.inventory;
        const auto statsPosition = absolute(layout.attributes);
        const auto& bonuses = summary.combinedBonuses;
        const auto& definition = battleContent_.combatants.at(currentPlayer());
        const auto& base = definition.attributes;
        const auto& resistances = definition.resistances;
        const std::array labels{"Gesundheit:",
                                "Rüstung:",
                                "Schaden:",
                                "Initiative:",
                                "Krit. Chance:",
                                "Ausweichen:",
                                "Präzision:",
                                "Schlaf-Res.:",
                                "Verwirrungs-Res.:",
                                "Debuff-Res.:",
                                "Verfall-Res.:"};
        const std::array values{std::to_string(base.maximumHealth + bonuses.maximumHealth),
                                std::to_string(base.armour + bonuses.armour),
                                std::to_string(base.damageMinimum + bonuses.damageMinimum) + "-" +
                                    std::to_string(base.damageMaximum + bonuses.damageMaximum),
                                std::to_string(base.initiative + bonuses.initiative),
                                std::to_string(base.criticalHitChance + bonuses.criticalHitChance) + "%",
                                std::to_string(base.dodge + bonuses.dodge),
                                std::to_string(base.precision + bonuses.precision),
                                std::to_string(resistances.sleepResistance + bonuses.sleepResistance) + "%",
                                std::to_string(resistances.confusionResistance + bonuses.confusionResistance) + "%",
                                std::to_string(resistances.debuffResistance + bonuses.debuffResistance) + "%",
                                std::to_string(resistances.decayResistance + bonuses.decayResistance) + "%"};
        float labelWidth{};
        for (const auto& label : labels)
        {
            labelWidth = std::max(labelWidth, detail::inventoryText(interfaceFont_, label, 16U, {}).getLocalBounds().size.x);
        }
        for (std::size_t index = 0; index < labels.size(); ++index)
        {
            const auto y = static_cast<float>(index) * 32.4F;
            auto label = detail::inventoryText(interfaceFont_, labels.at(index), 16U, statsPosition + sf::Vector2f{0.0F, y});
            label.setOutlineColor(sf::Color::Black);
            label.setOutlineThickness(1.0F);
            window_.draw(label);
            auto value = detail::inventoryText(interfaceFont_, values.at(index), 16U, statsPosition + sf::Vector2f{labelWidth + 10.0F, y},
                                       sf::Color{220, 220, 220});
            value.setOutlineColor(sf::Color::Black);
            value.setOutlineThickness(1.0F);
            window_.draw(value);
        }
    }

    void InventoryPanel::drawDiamondTooltip(const game::EquipmentBonusSummary& summary) const
    {
        const auto& layout = content_.presentation.inventory;
        const auto position = equipmentOrigin() + sf::Vector2f{static_cast<float>(layout.equipmentDiamond.x),
                                                               static_cast<float>(layout.equipmentDiamond.y)};
        const auto size = textures_.at("inventory.diamond").getSize();
        if (sf::FloatRect{position, sf::Vector2f{size}}.contains(mousePosition_))
        {
            drawBonusTooltip(window_, interfaceFont_, "Edelstein der Macht", summary.diamondBonuses, mousePosition_);
        }
    }

    void InventoryPanel::drawInventory() const
    {
        const auto panelPosition = absolute(content_.presentation.inventory.itemsPanel);
        sf::Sprite panel{textures_.at("inventory.items_panel")};
        panel.setPosition(panelPosition);
        window_.draw(panel);
        const auto entries = inventoryEntries();
        const auto first = scrollRow_ * 5U;
        const auto last = std::min(entries.size(), first + 25U);
        for (auto index = first; index < last; ++index)
        {
            if (dragged_ && !dragged_->equipmentSlot && dragged_->item == entries.at(index).item)
            {
                continue;
            }
            sf::Sprite item{textures_.at(entries.at(index).icon)};
            const auto visibleIndex = index - first;
            const auto position = panelPosition + sf::Vector2f{11.0F + static_cast<float>(visibleIndex % 5) * 116.0F,
                                                               11.0F + static_cast<float>(visibleIndex / 5) * 116.0F};
            sf::RectangleShape color{sf::Vector2f{item.getTexture().getSize()}};
            color.setPosition(position);
            const auto itemColor = gameplay::normalizedItemColor(entries.at(index).item.color);
            color.setFillColor({itemColor.red, itemColor.green, itemColor.blue});
            window_.draw(color);
            item.setPosition(position);
            window_.draw(item);
        }
        const auto knobBounds = scrollbarThumbBounds();
        sf::RectangleShape knob{knobBounds.size};
        knob.setPosition(knobBounds.position);
        knob.setFillColor(sf::Color{28, 28, 28});
        knob.setOutlineColor(sf::Color::Black);
        knob.setOutlineThickness(5.0F);
        window_.draw(knob);

        const auto buttonX = static_cast<float>(textures_.at("inventory.items_panel").getSize().x + 11U);
        window_.draw(economyButton(textures_.at("inventory.sort_name"), panelPosition + sf::Vector2f{buttonX, -14.0F},
                                   EconomyButtonBehavior::up, mousePosition_, mousePressed_));
        window_.draw(economyButton(textures_.at("inventory.sort_color"), panelPosition + sf::Vector2f{buttonX, 40.0F},
                                   EconomyButtonBehavior::up, mousePosition_, mousePressed_));
    }

    void InventoryPanel::drawDraggedItem() const
    {
        if (!dragged_)
        {
            return;
        }
        sf::Sprite item{textures_.at(content_.items.at(dragged_->item.definition).icon)};
        item.setPosition(mousePosition_ - sf::Vector2f{50.0F, 50.0F});
        sf::RectangleShape color{sf::Vector2f{item.getTexture().getSize()}};
        color.setPosition(item.getPosition());
        const auto itemColor = gameplay::normalizedItemColor(dragged_->item.color);
        color.setFillColor({itemColor.red, itemColor.green, itemColor.blue});
        window_.draw(color);
        window_.draw(item);
    }

    void InventoryPanel::drawTooltip() const
    {
        if (dragged_)
        {
            return;
        }
        std::optional<gameplay::ItemId> hovered;
        if (const auto index = inventoryIndexAt(mousePosition_))
        {
            hovered = inventoryEntries().at(*index).item.definition;
        }
        else if (const auto slot = equipmentSlotAt(mousePosition_))
        {
            if (const auto* equipped = state_.equipped(equipmentLocation(*slot)))
            {
                hovered = equipped->definition;
            }
        }
        if (!hovered)
        {
            return;
        }
        drawItemTooltip(window_, interfaceFont_, content_.items.at(*hovered), mousePosition_);
    }

    void InventoryPanel::drawStatTooltip() const
    {
        static constexpr std::array keys{"maximum_health",
                                         "armour",
                                         "damage",
                                         "initiative",
                                         "critical_hit",
                                         "dodge",
                                         "precision",
                                         "sleep_resistance",
                                         "confusion_resistance",
                                         "debuff_resistance",
                                         "decay_resistance"};
        const auto statsPosition = absolute(content_.presentation.inventory.attributes);
        for (std::size_t index = 0; index < keys.size(); ++index)
        {
            const auto rowPosition = statsPosition + sf::Vector2f{0.0F, static_cast<float>(index) * 32.4F - 4.0F};
            if (!sf::FloatRect{rowPosition, {310.0F, 28.0F}}.contains(mousePosition_))
            {
                continue;
            }
            const auto tooltip = battleContent_.statTooltips.find(keys.at(index));
            if (tooltip != battleContent_.statTooltips.end())
            {
                static_cast<void>(soma::presentation::drawTooltip(
                    window_, interfaceFont_, tooltip->second, mousePosition_ + sf::Vector2f{12.0F, -8.0F}, true, 430.0F));
            }
            return;
        }
    }
} // namespace soma::presentation
