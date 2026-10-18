#pragma once

#include "content/EconomyContent.hpp"
#include "game/CampaignState.hpp"
#include "game/EconomyQueries.hpp"
#include "game/EconomyService.hpp"
#include "game/EquipmentBonusCalculator.hpp"
#include "presentation/sfml/EconomyTextures.hpp"
#include "presentation/sfml/EconomySoundPlayer.hpp"
#include "presentation/sfml/EquipmentTransition.hpp"
#include "presentation/sfml/EquipmentConnectionRenderer.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <functional>
#include <optional>

namespace soma::presentation
{
    /// Provides inventory sorting, equipment drag-and-drop, derived stats, and connection feedback.
    class InventoryPanel
    {
      public:
        InventoryPanel(sf::RenderWindow& window,
                       const content::BattleContent& battleContent,
                       const content::EconomyContent& content,
                       game::CampaignState& state,
                       game::EconomyService& economy,
                       const EconomyTextures& textures,
                       const sf::Font& interfaceFont,
                       const sf::Font& titleFont,
                       bool closeWindowOnExit = true,
                       std::function<void()> drawBackground = {},
                       std::function<bool(sf::Vector2f)> navigate = {},
                       std::function<bool(sf::Vector2f)> navigationHovered = {});

        void processEvents();
        void render();
        [[nodiscard]] bool closeRequested() const noexcept;
        void showPlayer(std::size_t index);

      private:
        struct DraggedItem
        {
            gameplay::ItemInstance item;
            std::optional<std::size_t> equipmentSlot;
        };

        [[nodiscard]] gameplay::ContentDefinitionId currentPlayer() const;
        [[nodiscard]] game::EquipmentLocation equipmentLocation(std::size_t slot) const;
        [[nodiscard]] sf::Vector2f absolute(content::Point relative) const;
        [[nodiscard]] sf::Vector2f equipmentOrigin() const;
        [[nodiscard]] sf::FloatRect equipmentBounds(std::size_t slot) const;
        [[nodiscard]] std::vector<game::InventoryEntry> inventoryEntries() const;
        [[nodiscard]] std::size_t maximumScrollRow() const;
        [[nodiscard]] sf::FloatRect scrollbarTrackBounds() const;
        [[nodiscard]] sf::FloatRect scrollbarThumbBounds() const;
        [[nodiscard]] std::optional<std::size_t> inventoryIndexAt(sf::Vector2f position) const;
        [[nodiscard]] std::optional<std::size_t> equipmentSlotAt(sf::Vector2f position) const;
        void beginDrag(sf::Vector2f position);
        void finishDrag(sf::Vector2f position);
        void updateScrollFromMouse(float mouseY);
        void handleClick(sf::Vector2f position);
        void requestClose();
        void drawFrame() const;
        void drawEquipment() const;
        void drawEquipmentTopology(const game::EquipmentBonusSummary& summary) const;
        void drawEquippedItems() const;
        void drawEquipmentStatistics(const game::EquipmentBonusSummary& summary) const;
        void drawDiamondTooltip(const game::EquipmentBonusSummary& summary) const;
        void drawInventory() const;
        void drawDraggedItem() const;
        void drawTooltip() const;

        sf::RenderWindow& window_;
        const content::BattleContent& battleContent_;
        const content::EconomyContent& content_;
        game::CampaignState& state_;
        game::EconomyService& economy_;
        const EconomyTextures& textures_;
        const sf::Font& interfaceFont_;
        const sf::Font& titleFont_;
        std::size_t playerIndex_{};
        game::ItemSort sort_{game::ItemSort::name};
        std::size_t scrollRow_{};
        std::optional<DraggedItem> dragged_;
        sf::Vector2f mousePosition_{};
        bool mousePressed_{};
        bool scrollbarDragging_{};
        float scrollbarDragOffset_{};
        EconomySoundPlayer sounds_;
        mutable EquipmentTransition equipmentTransition_;
        EquipmentConnectionRenderer connectionRenderer_;
        bool closeWindowOnExit_{};
        bool closeRequested_{};
        std::function<void()> drawBackground_;
        std::function<bool(sf::Vector2f)> navigate_;
        std::function<bool(sf::Vector2f)> navigationHovered_;
        const std::array<std::string_view, 4> playerKeys_{"simon", "ole", "anna", "markus"};
        const std::array<std::string_view, 4> playerNames_{"Simon", "Ole", "Anna", "Markus"};
        const std::array<std::string_view, 4> slotKeys_{"north", "east", "south", "west"};
    };
} // namespace soma::presentation
