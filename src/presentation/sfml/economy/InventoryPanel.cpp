#include "presentation/sfml/economy/InventoryPanel.hpp"

#include "game/EquipmentBonusCalculator.hpp"
#include "presentation/sfml/battle/BattleHud.hpp"
#include "presentation/sfml/economy/EconomyTooltip.hpp"
#include "presentation/sfml/economy/EconomyButton.hpp"
#include "presentation/sfml/economy/InventoryPanelDrawing.hpp"
#include "presentation/sfml/common/UiCursor.hpp"

#include <algorithm>
#include <cmath>

namespace soma::presentation
{
    InventoryPanel::InventoryPanel(sf::RenderWindow& window,
                                   const content::BattleContent& battleContent,
                                   const content::EconomyContent& content,
                                   game::CampaignState& state,
                                   game::EconomyService& economy,
                                   const EconomyTextures& textures,
                                   const EconomyPanelFonts& fonts,
                                   PanelHostOptions host)
        : window_(window), battleContent_(battleContent), content_(content), state_(state), economy_(economy), textures_(textures),
          interfaceFont_(fonts.interface), titleFont_(fonts.title), connectionRenderer_(textures),
          closeWindowOnExit_(host.closeWindowOnExit), drawBackground_(std::move(host.drawBackground)), navigate_(std::move(host.navigate)),
          navigationHovered_(std::move(host.navigationHovered))
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
        drawStatTooltip();
        const auto& layout = content_.presentation.inventory;
        const auto panel = absolute(layout.itemsPanel);
        const auto panelWidth = textures_.at("inventory.items_panel").getSize().x;
        const auto buttonX = static_cast<float>(panelWidth + 11U);
        const auto overButton = detail::inventoryButtonSprite(textures_.at("skills.previous"), absolute(layout.previousButton), 2U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                detail::inventoryButtonSprite(textures_.at("skills.next"), absolute(layout.nextButton), 2U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                detail::inventoryButtonSprite(textures_.at("skills.close"), absolute(layout.closeButton), 3U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                detail::inventoryButtonSprite(textures_.at("inventory.sort_name"), panel + sf::Vector2f{buttonX, -14.0F}, 2U)
                                    .getGlobalBounds().contains(mousePosition_) ||
                                detail::inventoryButtonSprite(textures_.at("inventory.sort_color"), panel + sf::Vector2f{buttonX, 40.0F}, 2U)
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
        bool succeeded = false;
        if (!dragged_->equipmentSlot)
        {
            if (const auto destination = equipmentSlotAt(position))
            {
                succeeded = economy_.equip(equipmentLocation(*destination), dragged_->item).hasValue();
            }
        }
        else
        {
            if (const auto destination = equipmentSlotAt(position); destination && *destination != *dragged_->equipmentSlot)
            {
                succeeded = economy_.swapEquipment(equipmentLocation(*dragged_->equipmentSlot), equipmentLocation(*destination)).hasValue();
            }
            else
            {
                const auto inventoryPanel = absolute(content_.presentation.inventory.itemsPanel);
                const auto panelSize = textures_.at("inventory.items_panel").getSize();
                const sf::FloatRect bounds{inventoryPanel, {static_cast<float>(panelSize.x), static_cast<float>(panelSize.y)}};
                if (bounds.contains(position))
                {
                    succeeded = economy_.unequip(equipmentLocation(*dragged_->equipmentSlot)).hasValue();
                }
            }
        }

        if (succeeded)
        {
            sounds_.inventoryDrop();
            const auto currentSummary = game::EquipmentBonusCalculator{content_}.calculate(state_, currentPlayer());
            equipmentTransition_.start(previousSummary, currentSummary);
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
        const auto previous = detail::inventoryButtonSprite(textures_.at("skills.previous"), absolute(layout.previousButton), 2U).getGlobalBounds();
        const auto next = detail::inventoryButtonSprite(textures_.at("skills.next"), absolute(layout.nextButton), 2U).getGlobalBounds();
        const auto close = detail::inventoryButtonSprite(textures_.at("skills.close"), absolute(layout.closeButton), 3U).getGlobalBounds();
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
        const auto nameButton = detail::inventoryButtonSprite(textures_.at("inventory.sort_name"), itemsPanel + sf::Vector2f{buttonX, -14.0F}, 2U);
        const auto colorButton = detail::inventoryButtonSprite(textures_.at("inventory.sort_color"), itemsPanel + sf::Vector2f{buttonX, 40.0F}, 2U);
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
} // namespace soma::presentation
