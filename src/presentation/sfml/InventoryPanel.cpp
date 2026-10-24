#include "presentation/sfml/InventoryPanel.hpp"

#include "game/EquipmentBonusCalculator.hpp"
#include "presentation/sfml/EconomyTooltip.hpp"
#include "presentation/sfml/EconomyButton.hpp"
#include "presentation/sfml/UiCursor.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation
{
    namespace
    {
        sf::Text inventoryText(const sf::Font& font,
                               const std::string& value,
                               unsigned int size,
                               sf::Vector2f position,
                               sf::Color color = sf::Color::White)
        {
            sf::Text text{font, sf::String::fromUtf8(value.begin(), value.end()), size};
            text.setPosition(position);
            text.setFillColor(color);
            return text;
        }

        sf::Sprite buttonSprite(const sf::Texture& texture, sf::Vector2f position, unsigned int frames)
        {
            sf::Sprite sprite{texture};
            const auto width = texture.getSize().x / frames;
            sprite.setTextureRect(sf::IntRect{{0, 0}, {static_cast<int>(width), static_cast<int>(texture.getSize().y)}});
            sprite.setPosition(position);
            return sprite;
        }
    } // namespace

    InventoryPanel::InventoryPanel(sf::RenderWindow& window,
                                   const content::BattleContent& battleContent,
                                   const content::EconomyContent& content,
                                   game::CampaignState& state,
                                   game::EconomyService& economy,
                                   const EconomyTextures& textures,
                                   const sf::Font& interfaceFont,
                                   const sf::Font& titleFont,
                                   bool closeWindowOnExit,
                                   std::function<void()> drawBackground,
                                   std::function<bool(sf::Vector2f)> navigate,
                                   std::function<bool(sf::Vector2f)> navigationHovered)
        : window_(window), battleContent_(battleContent), content_(content), state_(state), economy_(economy), textures_(textures),
          interfaceFont_(interfaceFont),
          titleFont_(titleFont), connectionRenderer_(textures), closeWindowOnExit_(closeWindowOnExit),
          drawBackground_(std::move(drawBackground)), navigate_(std::move(navigate)),
          navigationHovered_(std::move(navigationHovered))
    {
    }

    void InventoryPanel::processEvents()
    {
        while (const auto event = window_.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window_.close();
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape)
            {
                requestClose();
            }
            if (const auto* moved = event->getIf<sf::Event::MouseMoved>())
            {
                mousePosition_ = window_.mapPixelToCoords(moved->position);
                if (scrollbarDragging_)
                {
                    updateScrollFromMouse(mousePosition_.y);
                }
            }
            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                const auto maximum = maximumScrollRow();
                if (wheel->delta > 0.0F)
                {
                    scrollRow_ = scrollRow_ == 0 ? 0 : scrollRow_ - 1;
                }
                else if (wheel->delta < 0.0F)
                {
                    scrollRow_ = std::min(maximum, scrollRow_ + 1);
                }
            }
            if (const auto* press = event->getIf<sf::Event::MouseButtonPressed>(); press && press->button == sf::Mouse::Button::Left)
            {
                mousePressed_ = true;
                mousePosition_ = window_.mapPixelToCoords(press->position);
                const auto thumb = scrollbarThumbBounds();
                if (thumb.contains(mousePosition_))
                {
                    scrollbarDragging_ = true;
                    scrollbarDragOffset_ = mousePosition_.y - thumb.position.y;
                }
                else if (maximumScrollRow() > 0U && scrollbarTrackBounds().contains(mousePosition_))
                {
                    scrollbarDragging_ = true;
                    scrollbarDragOffset_ = thumb.size.y / 2.0F;
                    updateScrollFromMouse(mousePosition_.y);
                }
                else
                {
                    beginDrag(mousePosition_);
                }
            }
            if (const auto* release = event->getIf<sf::Event::MouseButtonReleased>(); release && release->button == sf::Mouse::Button::Left)
            {
                mousePressed_ = false;
                mousePosition_ = window_.mapPixelToCoords(release->position);
                const auto wasDragging = dragged_.has_value() || scrollbarDragging_;
                finishDrag(mousePosition_);
                scrollbarDragging_ = false;
                if (!wasDragging && navigate_ && navigate_(mousePosition_))
                {
                    requestClose();
                }
                else if (!wasDragging)
                {
                    handleClick(mousePosition_);
                }
            }
        }
    }

    bool InventoryPanel::closeRequested() const noexcept
    {
        return closeRequested_;
    }

    void InventoryPanel::showPlayer(std::size_t index)
    {
        playerIndex_ = index % playerKeys_.size();
        equipmentTransition_.reset();
    }

    void InventoryPanel::requestClose()
    {
        closeRequested_ = true;
        if (closeWindowOnExit_)
        {
            window_.close();
        }
    }

    void InventoryPanel::render()
    {
        window_.clear(sf::Color{30, 30, 30});
        if (drawBackground_) drawBackground_();
        drawFrame();
        drawEquipment();
        drawInventory();
        drawDraggedItem();
        drawTooltip();
        const auto& layout = content_.presentation.inventory;
        const auto panel = absolute(layout.itemsPanel);
        const auto panelWidth = textures_.at("inventory.items_panel").getSize().x;
        const auto buttonX = static_cast<float>(panelWidth + 11U);
        const auto overButton = buttonSprite(textures_.at("skills.previous"), absolute(layout.previousButton), 2U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                buttonSprite(textures_.at("skills.next"), absolute(layout.nextButton), 2U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                buttonSprite(textures_.at("skills.close"), absolute(layout.closeButton), 3U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                buttonSprite(textures_.at("inventory.sort_name"), panel + sf::Vector2f{buttonX, -14.0F}, 2U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                buttonSprite(textures_.at("inventory.sort_color"), panel + sf::Vector2f{buttonX, 40.0F}, 2U)
                                    .getGlobalBounds().contains(mousePosition_);
        setUiCursor(window_, overButton || scrollbarTrackBounds().contains(mousePosition_) || inventoryIndexAt(mousePosition_).has_value() ||
                                 equipmentSlotAt(mousePosition_).has_value() ||
                                 (navigationHovered_ && navigationHovered_(mousePosition_)));
        window_.display();
    }

    gameplay::ContentDefinitionId InventoryPanel::currentPlayer() const
    {
        return gameplay::ContentDefinitionId{"player." + std::string{playerKeys_.at(playerIndex_)}};
    }

    game::EquipmentLocation InventoryPanel::equipmentLocation(std::size_t slot) const
    {
        return {currentPlayer(), gameplay::EquipmentSlotId{"equipment." + std::string{slotKeys_.at(slot)}}};
    }

    sf::Vector2f InventoryPanel::absolute(content::Point relative) const
    {
        const auto origin = content_.presentation.inventory.origin;
        return {static_cast<float>(origin.x + relative.x), static_cast<float>(origin.y + relative.y)};
    }

    sf::Vector2f InventoryPanel::equipmentOrigin() const
    {
        return absolute(content_.presentation.inventory.equipmentPanel);
    }

    sf::FloatRect InventoryPanel::equipmentBounds(std::size_t slot) const
    {
        const auto field = content_.presentation.inventory.equipmentFields.at(slot);
        const auto position = equipmentOrigin() + sf::Vector2f{static_cast<float>(field.x), static_cast<float>(field.y)};
        const auto size = textures_.at("inventory.equipment_field").getSize();
        return {position, {static_cast<float>(size.x), static_cast<float>(size.y)}};
    }

    std::vector<game::InventoryEntry> InventoryPanel::inventoryEntries() const
    {
        return game::EconomyQueries{state_, content_}.inventory(sort_);
    }

    std::size_t InventoryPanel::maximumScrollRow() const
    {
        const auto rows = (inventoryEntries().size() + 4U) / 5U;
        return rows > 5U ? rows - 5U : 0U;
    }

    sf::FloatRect InventoryPanel::scrollbarTrackBounds() const
    {
        const auto panel = absolute(content_.presentation.inventory.itemsPanel);
        const auto& bounds = content_.presentation.inventory.itemsScrollbar;
        return {panel + sf::Vector2f{static_cast<float>(bounds.x), static_cast<float>(bounds.y)},
                {static_cast<float>(bounds.width), static_cast<float>(bounds.height)}};
    }

    sf::FloatRect InventoryPanel::scrollbarThumbBounds() const
    {
        const auto track = scrollbarTrackBounds();
        const auto steps = maximumScrollRow() + 1U;
        const auto height = std::max(10.0F, track.size.y / static_cast<float>(steps) - 10.0F);
        const auto progress = steps == 1U ? 0.0F : static_cast<float>(scrollRow_) / static_cast<float>(steps - 1U);
        return {track.position + sf::Vector2f{0.0F, progress * (track.size.y - height)}, {track.size.x, height}};
    }

    std::optional<std::size_t> InventoryPanel::inventoryIndexAt(sf::Vector2f position) const
    {
        const auto entries = inventoryEntries();
        const auto panel = absolute(content_.presentation.inventory.itemsPanel);
        const auto first = scrollRow_ * 5U;
        const auto last = std::min(entries.size(), first + 25U);
        for (auto index = first; index < last; ++index)
        {
            const auto visibleIndex = index - first;
            const sf::FloatRect bounds{panel + sf::Vector2f{11.0F + static_cast<float>(visibleIndex % 5) * 116.0F,
                                                           11.0F + static_cast<float>(visibleIndex / 5) * 116.0F},
                                       {100.0F, 100.0F}};
            if (bounds.contains(position))
            {
                return index;
            }
        }
        return std::nullopt;
    }

    std::optional<std::size_t> InventoryPanel::equipmentSlotAt(sf::Vector2f position) const
    {
        for (std::size_t slot = 0; slot < 4; ++slot)
        {
            if (equipmentBounds(slot).contains(position))
            {
                return slot;
            }
        }
        return std::nullopt;
    }

    void InventoryPanel::beginDrag(sf::Vector2f position)
    {
        if (const auto inventoryIndex = inventoryIndexAt(position))
        {
            dragged_ = DraggedItem{inventoryEntries().at(*inventoryIndex).item, std::nullopt};
            return;
        }
        if (const auto slot = equipmentSlotAt(position))
        {
            if (const auto* equipped = state_.equipped(equipmentLocation(*slot)))
            {
                dragged_ = DraggedItem{*equipped, slot};
            }
        }
    }

    void InventoryPanel::finishDrag(sf::Vector2f position)
    {
        if (!dragged_)
        {
            return;
        }
        const auto previousSummary = game::EquipmentBonusCalculator{content_}.calculate(state_, currentPlayer());
        if (!dragged_->equipmentSlot)
        {
            if (const auto destination = equipmentSlotAt(position))
            {
                const auto result = economy_.equip(equipmentLocation(*destination), dragged_->item);
                if (result.hasValue())
                {
                    sounds_.inventoryDrop();
                    const auto currentSummary = game::EquipmentBonusCalculator{content_}.calculate(state_, currentPlayer());
                    equipmentTransition_.start(previousSummary, currentSummary);
                }
            }
        }
        else
        {
            if (const auto destination = equipmentSlotAt(position); destination && *destination != *dragged_->equipmentSlot)
            {
                const auto result = economy_.swapEquipment(equipmentLocation(*dragged_->equipmentSlot), equipmentLocation(*destination));
                if (result.hasValue())
                {
                    sounds_.inventoryDrop();
                    const auto currentSummary = game::EquipmentBonusCalculator{content_}.calculate(state_, currentPlayer());
                    equipmentTransition_.start(previousSummary, currentSummary);
                }
                dragged_.reset();
                return;
            }
            const auto inventoryPanel = absolute(content_.presentation.inventory.itemsPanel);
            const auto panelSize = textures_.at("inventory.items_panel").getSize();
            const sf::FloatRect bounds{inventoryPanel, {static_cast<float>(panelSize.x), static_cast<float>(panelSize.y)}};
            if (bounds.contains(position))
            {
                const auto result = economy_.unequip(equipmentLocation(*dragged_->equipmentSlot));
                if (result.hasValue())
                {
                    sounds_.inventoryDrop();
                    const auto currentSummary = game::EquipmentBonusCalculator{content_}.calculate(state_, currentPlayer());
                    equipmentTransition_.start(previousSummary, currentSummary);
                }
            }
        }
        dragged_.reset();
    }

    void InventoryPanel::updateScrollFromMouse(float mouseY)
    {
        const auto maximum = maximumScrollRow();
        if (maximum == 0U)
        {
            scrollRow_ = 0U;
            return;
        }
        const auto track = scrollbarTrackBounds();
        const auto thumb = scrollbarThumbBounds();
        const auto travel = track.size.y - thumb.size.y;
        const auto progress = std::clamp((mouseY - scrollbarDragOffset_ - track.position.y) / travel, 0.0F, 1.0F);
        scrollRow_ = static_cast<std::size_t>(std::round(progress * static_cast<float>(maximum)));
    }

    void InventoryPanel::handleClick(sf::Vector2f position)
    {
        const auto& layout = content_.presentation.inventory;
        const auto previous = buttonSprite(textures_.at("skills.previous"), absolute(layout.previousButton), 2U).getGlobalBounds();
        const auto next = buttonSprite(textures_.at("skills.next"), absolute(layout.nextButton), 2U).getGlobalBounds();
        const auto close = buttonSprite(textures_.at("skills.close"), absolute(layout.closeButton), 3U).getGlobalBounds();
        if (previous.contains(position))
        {
            sounds_.click();
            playerIndex_ = playerIndex_ == 0 ? playerKeys_.size() - 1 : playerIndex_ - 1;
            equipmentTransition_.reset();
        }
        else if (next.contains(position))
        {
            sounds_.click();
            playerIndex_ = (playerIndex_ + 1) % playerKeys_.size();
            equipmentTransition_.reset();
        }
        else if (close.contains(position))
        {
            sounds_.click();
            requestClose();
        }
        const auto itemsPanel = absolute(layout.itemsPanel);
        const auto panelWidth = textures_.at("inventory.items_panel").getSize().x;
        const auto buttonX = static_cast<float>(panelWidth + 11U);
        const auto nameButton = buttonSprite(textures_.at("inventory.sort_name"), itemsPanel + sf::Vector2f{buttonX, -14.0F}, 2U);
        const auto colorButton = buttonSprite(textures_.at("inventory.sort_color"), itemsPanel + sf::Vector2f{buttonX, 40.0F}, 2U);
        if (nameButton.getGlobalBounds().contains(position))
        {
            sounds_.click();
            sort_ = game::ItemSort::name;
            scrollRow_ = 0;
        }
        else if (colorButton.getGlobalBounds().contains(position))
        {
            sounds_.click();
            sort_ = game::ItemSort::color;
            scrollRow_ = 0;
        }
    }

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
        window_.draw(inventoryText(titleFont_, "Inventar", 70U, absolute({850, 70}), sf::Color::Black));
        auto player = inventoryText(titleFont_, std::string{playerNames_.at(playerIndex_)}, 47U, {}, sf::Color::Black);
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
        const std::array labels{"Rüstung:",
                                "Schaden:",
                                "Initiative:",
                                "Krit. Chance:",
                                "Ausweichen:",
                                "Präzision:",
                                "Heilungsmod.:",
                                "Schlaf-Res.:",
                                "Verwirrungs-Res.:",
                                "Debuff-Res.:",
                                "Verfall-Res.:"};
        const std::array values{std::to_string(base.armour + bonuses.armour),
                                std::to_string(base.damageMinimum + bonuses.damageMinimum) + "-" +
                                    std::to_string(base.damageMaximum + bonuses.damageMaximum),
                                std::to_string(base.initiative + bonuses.initiative),
                                std::to_string(base.criticalHitChance + bonuses.criticalHitChance) + "%",
                                std::to_string(base.dodge + bonuses.dodge),
                                std::to_string(base.precision + bonuses.precision),
                                std::to_string(resistances.healing + bonuses.healing) + "%",
                                std::to_string(resistances.sleepResistance + bonuses.sleepResistance) + "%",
                                std::to_string(resistances.confusionResistance + bonuses.confusionResistance) + "%",
                                std::to_string(resistances.debuffResistance + bonuses.debuffResistance) + "%",
                                std::to_string(resistances.decayResistance + bonuses.decayResistance) + "%"};
        float labelWidth{};
        for (const auto& label : labels)
        {
            labelWidth = std::max(labelWidth, inventoryText(interfaceFont_, label, 16U, {}).getLocalBounds().size.x);
        }
        for (std::size_t index = 0; index < labels.size(); ++index)
        {
            const auto y = static_cast<float>(index) * 32.4F;
            auto label = inventoryText(interfaceFont_, labels.at(index), 16U, statsPosition + sf::Vector2f{0.0F, y});
            label.setOutlineColor(sf::Color::Black);
            label.setOutlineThickness(1.0F);
            window_.draw(label);
            auto value = inventoryText(interfaceFont_, values.at(index), 16U, statsPosition + sf::Vector2f{labelWidth + 10.0F, y},
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
} // namespace soma::presentation
