#pragma once

#include "content/economy/EconomyContent.hpp"
#include "game/CampaignState.hpp"
#include "game/EconomyService.hpp"
#include "game/RuleBuilderService.hpp"
#include "presentation/sfml/economy/EconomySoundPlayer.hpp"
#include "presentation/sfml/economy/EconomyTextures.hpp"
#include "presentation/sfml/economy/EconomyPanelOptions.hpp"

#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <random>
#include <unordered_map>

namespace soma::presentation
{
    /// Handles buying, selling, stock selection, and permanent shop upgrades.
    class ShopPanel
    {
      public:
        ShopPanel(sf::RenderWindow& window,
                  const content::EconomyContent& content,
                  const content::RuleContent& rules,
                  game::CampaignState& state,
                  game::EconomyService& economy,
                  game::RuleBuilderService& ruleService,
                  const EconomyTextures& textures,
                  const EconomyPanelFonts& fonts,
                  PanelHostOptions host = {});

        void processEvents();
        void render();
        [[nodiscard]] bool closeRequested() const noexcept;

      private:
        enum class SelectionSource
        {
            equipmentOffer,
            consumableOffer,
            dice,
            inventory,
            consumableInventory,
            ruleOffer
        };

        struct Selection
        {
            SelectionSource source{};
            gameplay::ItemInstance item;
            std::optional<gameplay::RuleFragmentId> fragment;
        };

        enum class Dialog
        {
            none,
            unlockSlot,
            reduceDicePrice,
            sellMultiple
        };

        [[nodiscard]] sf::Vector2f vendingPosition(content::Point relative) const;
        [[nodiscard]] sf::Vector2f shopPosition(content::Point relative) const;
        [[nodiscard]] std::vector<gameplay::ItemId> consumableDefinitions() const;
        [[nodiscard]] sf::FloatRect equipmentOfferBounds(std::size_t index) const;
        [[nodiscard]] sf::FloatRect consumableOfferBounds(std::size_t index) const;
        [[nodiscard]] sf::FloatRect inventoryBounds(std::size_t index) const;
        [[nodiscard]] sf::FloatRect consumableInventoryBounds(std::size_t index) const;
        [[nodiscard]] std::optional<sf::Vector2f> selectionPosition() const;
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;
        [[nodiscard]] std::size_t visibleShopOfferCount() const noexcept;
        void refillShopStock();
        void handleClick(sf::Vector2f position);
        void requestClose();
        void handleDialogClick(sf::Vector2f position);
        void handleSellMultipleDialog(sf::Vector2f position, sf::Vector2f panelPosition);
        [[nodiscard]] sf::FloatRect sellSliderBounds(sf::Vector2f panelPosition) const;
        void updateSaleQuantity(sf::Vector2f position, const sf::FloatRect& sliderBounds);
        void handleConfirmationDialog(sf::Vector2f position, sf::Vector2f panelPosition);
        void selectAt(sf::Vector2f position);
        void executeSelection();
        void sellSelectedConsumable();
        void drawPanels() const;
        void drawSelection() const;
        void drawShopOffers() const;
        void drawOwnedItems() const;
        void drawControls() const;
        void drawTooltip() const;
        void drawDialog() const;
        void drawSellMultipleDialog(sf::Vector2f position) const;
        void drawConfirmationDialog(sf::Vector2f position) const;
        void drawDialogButton(std::string_view label, sf::Vector2f position, unsigned int characterSize) const;
        [[nodiscard]] sf::Vector2f dialogPosition(const sf::Texture& texture) const;
        void drawItem(const gameplay::ItemInstance& item, sf::Vector2f position, bool dimmed = false) const;
        void drawPrice(int price, sf::Vector2f itemPosition) const;

        sf::RenderWindow& window_;
        const content::EconomyContent& content_;
        const content::RuleContent& rules_;
        game::CampaignState& state_;
        game::EconomyService& economy_;
        game::RuleBuilderService& ruleService_;
        const EconomyTextures& textures_;
        const sf::Font& interfaceFont_;
        const sf::Font& titleFont_;
        const sf::Font& priceFont_;
        std::mt19937 random_{std::random_device{}()};
        std::unordered_map<content::RuleFragmentType, sf::Texture> ruleTextures_;
        std::optional<Selection> selection_;
        Dialog dialog_{Dialog::none};
        int saleQuantity_{1};
        std::size_t inventoryScrollRow_{};
        sf::Vector2f mousePosition_{};
        bool mousePressed_{};
        bool sliderDragging_{};
        EconomySoundPlayer sounds_;
        bool closeWindowOnExit_{};
        bool closeRequested_{};
        std::function<void()> drawBackground_;
        std::function<bool(sf::Vector2f)> navigate_;
        std::function<bool(sf::Vector2f)> navigationHovered_;
    };
} // namespace soma::presentation
