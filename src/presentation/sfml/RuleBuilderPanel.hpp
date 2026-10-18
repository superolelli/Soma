#pragma once

#include "content/RuleContent.hpp"
#include "game/RuleBuilderService.hpp"

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <functional>

namespace soma::presentation
{
    /// Presents the rule-fragment library and edits the campaign's constructible game rules.
    class RuleBuilderPanel
    {
      public:
        RuleBuilderPanel(sf::RenderWindow& window,
                         const content::RuleContent& content,
                         game::CampaignState& state,
                         const sf::Font& font,
                         const sf::Font& titleFont,
                         std::function<void()> drawBackground = {},
                         std::function<bool(sf::Vector2f)> navigate = {},
                         std::function<bool(sf::Vector2f)> navigationHovered = {});

        void processEvents();
        void render();
        [[nodiscard]] bool closeRequested() const noexcept;

      private:
        struct LibraryEntry
        {
            const content::RuleFragmentDefinition* definition{};
            int count{};
        };

        void handleMouseMove(const sf::Event::MouseMoved& event);
        void handleMousePress(const sf::Event::MouseButtonPressed& event);
        void handleMouseRelease(const sf::Event::MouseButtonReleased& event);
        void beginFragmentDrag(sf::Vector2f position);
        void finishFragmentDrag(sf::Vector2f position);
        void tryOpenSlotUnlock(sf::Vector2f position);
        [[nodiscard]] sf::FloatRect fragmentPoolBounds() const;
        [[nodiscard]] std::vector<LibraryEntry> library() const;
        [[nodiscard]] sf::FloatRect libraryBounds(std::size_t visibleIndex) const;
        [[nodiscard]] sf::FloatRect partBounds(std::size_t slot, content::RuleFragmentType type) const;
        [[nodiscard]] std::vector<content::RuleFragmentType> visibleTypes(std::size_t slot) const;
        [[nodiscard]] std::optional<gameplay::RuleFragmentId> fragmentAt(sf::Vector2f position) const;
        [[nodiscard]] std::optional<std::pair<std::size_t, content::RuleFragmentType>> partAt(sf::Vector2f position) const;
        [[nodiscard]] std::optional<gameplay::RuleFragmentId> fragmentInPart(const std::pair<std::size_t, content::RuleFragmentType>& part) const;
        [[nodiscard]] std::optional<std::size_t> ruleAt(sf::Vector2f position) const;
        [[nodiscard]] sf::FloatRect scrollbarThumb(const std::vector<LibraryEntry>& entries) const;
        void updateScrollFromMouse(float mouseY);
        [[nodiscard]] std::string ruleSentence(std::size_t slot) const;
        void drawConfirmation(sf::Vector2f mouse) const;
        bool handleConfirmationClick(sf::Vector2f position);
        void drawLibrary(sf::Vector2f mouse) const;
        void drawRules(sf::Vector2f mouse) const;
        void drawPart(std::size_t slot, content::RuleFragmentType type, sf::Vector2f mouse) const;
        void drawText(std::string_view value, sf::Vector2f position, unsigned int size, sf::Color color = sf::Color::Black) const;
        [[nodiscard]] sf::Color color(content::RuleFragmentType type, std::uint8_t alpha = 220) const;

        sf::RenderWindow& window_;
        const content::RuleContent& content_;
        game::CampaignState& state_;
        game::RuleBuilderService service_;
        const sf::Font& font_;
        const sf::Font& titleFont_;
        std::function<void()> drawBackground_;
        std::function<bool(sf::Vector2f)> navigate_;
        std::function<bool(sf::Vector2f)> navigationHovered_;
        sf::Texture panel_;
        sf::Texture fragmentBackground_;
        sf::Texture slotLock_;
        sf::Texture closeButton_;
        sf::Texture confirmationDialog_;
        sf::Texture confirmationButton_;
        std::unordered_map<content::RuleFragmentType, sf::Texture> sigils_;
        int scroll_{};
        std::optional<gameplay::RuleFragmentId> dragged_;
        std::optional<std::pair<std::size_t, content::RuleFragmentType>> draggedFromPart_;
        sf::Vector2f dragPosition_;
        float scrollbarDragOffset_{};
        std::string message_;
        bool scrollbarDragging_{};
        bool confirmingUnlock_{};
        bool closeRequested_{};
    };
} // namespace soma::presentation
