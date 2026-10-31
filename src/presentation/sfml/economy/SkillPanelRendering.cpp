#include "presentation/sfml/economy/SkillPanel.hpp"

#include "game/CampaignBattleContentBuilder.hpp"
#include "presentation/sfml/economy/EconomyButton.hpp"
#include "presentation/sfml/common/FormattedTextLayout.hpp"
#include "presentation/sfml/economy/SkillPanelLayout.hpp"
#include "presentation/sfml/common/StatusGlossary.hpp"
#include "presentation/text/AbilityDescription.hpp"
#include "presentation/text/SkillDescription.hpp"

#include <algorithm>
#include <sstream>

namespace soma::presentation
{
    namespace
    {
        sf::Text makeText(const sf::Font& font,
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

        void centerText(sf::Text& text, const sf::FloatRect& bounds)
        {
            const auto textBounds = text.getLocalBounds();
            text.setPosition(bounds.getCenter() - textBounds.position - textBounds.size / 2.0F);
        }

        std::string wrapText(const sf::Font& font, const std::string& value, float maximumWidth, unsigned int characterSize)
        {
            std::istringstream words{value};
            std::string result;
            std::string line;
            std::string word;
            while (words >> word)
            {
                const auto candidate = line.empty() ? word : line + ' ' + word;
                if (!line.empty() && makeText(font, candidate, characterSize, {}).getLocalBounds().size.x > maximumWidth)
                {
                    if (!result.empty()) result += '\n';
                    result += line;
                    line = std::move(word);
                }
                else
                {
                    line = std::move(candidate);
                }
            }
            if (!line.empty())
            {
                if (!result.empty()) result += '\n';
                result += line;
            }
            return result;
        }

        std::string skillTooltip(const game::SkillEntry& skill)
        {
            return skillDescription(skill.name, skill.description);
        }

        sf::Vector2f clampedTooltipPosition(sf::Vector2f preferred, sf::Vector2f size, sf::Vector2f screenSize)
        {
            return {std::clamp(preferred.x, 5.0F, std::max(5.0F, screenSize.x - size.x - 5.0F)),
                    std::clamp(preferred.y, 5.0F, std::max(5.0F, screenSize.y - size.y - 5.0F))};
        }
    } // namespace
    void SkillPanel::drawBackground() const
    {
        sf::Sprite panel{textures_.at("skills.panel")};
        panel.setPosition(absolute({0, 0}));
        window_.draw(panel);
        const auto& layout = content_.presentation.skills;
        window_.draw(economyButton(textures_.at("skills.previous"), absolute(layout.previousButton), EconomyButtonBehavior::up,
                                   mousePosition_, mousePressed_));
        window_.draw(economyButton(textures_.at("skills.next"), absolute(layout.nextButton), EconomyButtonBehavior::up,
                                   mousePosition_, mousePressed_));
        window_.draw(economyButton(textures_.at("skills.close"), absolute(layout.closeButton), EconomyButtonBehavior::motionUp,
                                   mousePosition_, mousePressed_));
        auto title = makeText(titleFont_, "Fähigkeiten", 62U, {}, sf::Color::Black);
        centerText(title, absoluteBounds(layout.panelTitleBounds));
        window_.draw(title);
    }

    void SkillPanel::drawAbilityBridge() const
    {
        sf::Sprite bridge{textures_.at("skills.bridge")};
        bridge.setPosition(absolute(content_.presentation.skills.abilityBridges.at(abilityIndex_)));
        window_.draw(bridge);
    }

    void SkillPanel::drawPlayerAndAbilities() const
    {
        const auto& layout = content_.presentation.skills;
        auto player = makeText(titleFont_, std::string{playerNames_.at(playerIndex_)}, 47U, {}, sf::Color::Black);
        centerText(player, absoluteBounds(layout.playerNameBounds));
        window_.draw(player);

        for (std::size_t index = 0; index < 4; ++index)
        {
            const auto* set = game::EconomyQueries{state_, content_}.abilitySet(currentPlayerId(), index);
            if (set == nullptr) continue;
            const auto abilityId = set->versions.at(state_.activeAbilityVersion(currentPlayerId(), index)).ability;
            auto iconBounds = layout.abilityIconBounds;
            iconBounds.y = layout.abilityHitboxes.at(index).y;
            sf::Sprite icon{textures_.at(abilityId.value())};
            icon.setPosition(absoluteBounds(iconBounds).getCenter() - sf::Vector2f{icon.getTexture().getSize()} / 2.0F);
            if (index != abilityIndex_) icon.setColor(sf::Color{180, 180, 180});
            window_.draw(icon);
            auto nameBounds = layout.abilityNameBounds;
            nameBounds.y = layout.abilityHitboxes.at(index).y;
            const auto wrappedName = wrapText(abilityFont_, battleContent_.abilities.at(abilityId).name,
                                              static_cast<float>(nameBounds.width - 20), 24U);
            auto name = makeText(abilityFont_, wrappedName, 24U, {}, sf::Color::Black);
            centerText(name, absoluteBounds(nameBounds));
            window_.draw(name);
        }

        for (std::size_t version = 0; version < 2; ++version)
        {
            const auto root = detail::addPoints(layout.treeOrigins.at(version), layout.versionAbilityIcon);
            if (version == selectedVersion_)
            {
                sf::Sprite highlight{textures_.at("skills.acquired")};
                highlight.setPosition(absolute(detail::addPoints(root, layout.acquiredInset)));
                window_.draw(highlight);
            }
            sf::Sprite icon{textures_.at(currentAbilityId(version).value())};
            icon.setPosition(absolute(root));
            window_.draw(icon);
        }
    }

    void SkillPanel::drawSkillGraph() const
    {
        const auto& layout = content_.presentation.skills;
        const auto activeVersion = state_.activeAbilityVersion(currentPlayerId(), abilityIndex_);
        for (std::size_t version = 0; version < 2; ++version)
        {
            const auto tree = layout.treeOrigins.at(version);
            const auto active = version == activeVersion;
            const auto selectorAsset = active ? "skills.ability_active" : "skills.ability_inactive";
            sf::Sprite selector{textures_.at(selectorAsset)};
            selector.setPosition(absolute(detail::addPoints(tree, layout.versionSelectors.at(version))));
            const auto mustFaceRight = version == 0;
            const auto alreadyFacesRight = active;
            if (mustFaceRight != alreadyFacesRight)
            {
                selector.setScale({-1.0F, 1.0F});
                selector.move({static_cast<float>(selector.getTexture().getSize().x), 0.0F});
            }
            window_.draw(selector);

            const auto skills = currentSkills(version);
            for (std::size_t connection = 0; connection < layout.connectionOffsets.size(); ++connection)
            {
                const auto acquired = connection == 0 ||
                                      (connection == 1 && skills.at(0).availability == game::SkillAvailability::acquired) ||
                                      (connection == 2 && skills.at(1).availability == game::SkillAvailability::acquired) ||
                                      (connection == 3 && skills.at(2).availability == game::SkillAvailability::acquired &&
                                       skills.at(3).availability == game::SkillAvailability::acquired);
                const auto asset = "skills.connection." + std::to_string(layout.connectionTypes.at(connection)) +
                                   (acquired ? ".acquired" : ".locked");
                sf::Sprite line{textures_.at(asset)};
                line.setPosition(absolute(detail::addPoints(tree, layout.connectionOffsets.at(connection))));
                window_.draw(line);
            }
            sf::Sprite placeholders{textures_.at("skills.placeholders")};
            placeholders.setPosition(absolute(tree));
            window_.draw(placeholders);
        }
    }

    void SkillPanel::drawSkills() const
    {
        const auto& layout = content_.presentation.skills;
        for (std::size_t version = 0; version < 2; ++version)
        {
            const auto skills = currentSkills(version);
            const auto tree = layout.treeOrigins.at(version);
            for (std::size_t index = 0; index < skills.size(); ++index)
            {
                const auto position = absolute(detail::addPoints(tree, layout.skillIcons.at(index)));
                if (skills.at(index).availability == game::SkillAvailability::acquired)
                {
                    sf::Sprite acquired{textures_.at("skills.acquired")};
                    acquired.setPosition(absolute(detail::addPoints(detail::addPoints(tree, layout.skillIcons.at(index)), layout.acquiredInset)));
                    window_.draw(acquired);
                }
                sf::Sprite icon{textures_.at(skills.at(index).icon)};
                icon.setPosition(position);
                if (skills.at(index).availability == game::SkillAvailability::locked) icon.setColor(sf::Color{40, 40, 40});
                window_.draw(icon);
            }
        }
        const auto position = detail::addPoints(content_.presentation.skills.treeOrigins.at(selectedVersion_),
                                  content_.presentation.skills.skillIcons.at(skillIndex_));
        sf::Sprite selection{textures_.at("skills.selection")};
        selection.setPosition(absolute(detail::addPoints(position, content_.presentation.skills.selectionInset)));
        window_.draw(selection);
    }

    void SkillPanel::drawSelectionDetails() const
    {
        const auto skills = currentSkills(selectedVersion_);
        if (skills.empty()) return;
        const auto& selected = skills.at(std::min(skillIndex_, skills.size() - 1));
        const auto& layout = content_.presentation.skills;
        const auto wrappedTitle = wrapText(abilityFont_, selected.name, static_cast<float>(layout.selectedSkillTitleBounds.width - 20), 25U);
        auto title = makeText(abilityFont_, wrappedTitle, 25U, {}, sf::Color::Black);
        centerText(title, absoluteBounds(layout.selectedSkillTitleBounds));
        window_.draw(title);
        if (selected.availability == game::SkillAvailability::acquired) return;

        sf::Sprite dice{textures_.at("skills.dice")};
        dice.setPosition(absolute(layout.dice));
        window_.draw(dice);
        window_.draw(makeText(priceFont_, std::to_string(selected.cost), 26U,
                              absolute({layout.dice.x + layout.diceCostOffset.x, layout.dice.y + layout.diceCostOffset.y})));
        window_.draw(economyButton(textures_.at("skills.buy"), absolute(layout.buyButton), EconomyButtonBehavior::motionUp,
                                   mousePosition_, mousePressed_, canBuy(selected)));
        auto label = makeText(interfaceFont_, "Kaufen", 30U, {}, sf::Color::Black);
        const sf::Vector2f buttonSize{static_cast<float>(textures_.at("skills.buy").getSize().x / 3U),
                                      static_cast<float>(textures_.at("skills.buy").getSize().y)};
        centerText(label, {absolute(layout.buyButton), buttonSize});
        window_.draw(label);
    }

    void SkillPanel::drawHoveredSkillTooltip() const
    {
        const auto& layout = content_.presentation.skills;
        for (std::size_t version = 0; version < 2; ++version)
        {
            const auto skills = currentSkills(version);
            const auto tree = layout.treeOrigins.at(version);
            for (std::size_t index = 0; index < skills.size(); ++index)
            {
                const auto position = absolute(detail::addPoints(tree, layout.skillIcons.at(index)));
                if (!sf::FloatRect{position, sf::Vector2f{textures_.at(skills.at(index).icon).getSize()}}.contains(mousePosition_)) continue;
                const auto primaryBounds = drawTooltip(skillTooltip(skills.at(index)), mousePosition_ + sf::Vector2f{18.0F, -120.0F});
                const auto statusDetails = skillStatusDetails(content_.skills.at(skills.at(index).skill).upgrade);
                if (!statusDetails.empty())
                {
                    const auto referenceSize = sf::Vector2f{static_cast<float>(content_.presentation.referenceSize.x),
                                                            static_cast<float>(content_.presentation.referenceSize.y)};
                    static_cast<void>(drawStatusGlossary(window_,
                                                         interfaceFont_,
                                                         statusDetails,
                                                         [this](const auto& status) { return &textures_.at(status.value()); },
                                                         {referenceSize.x - 20.0F, 70.0F},
                                                         primaryBounds));
                }
                return;
            }
        }
    }

    void SkillPanel::drawSelectedAbilityTooltip() const
    {
        const auto& layout = content_.presentation.skills;
        for (std::size_t version = 0; version < 2; ++version)
        {
            const auto iconPosition = absolute(detail::addPoints(layout.treeOrigins.at(version), layout.versionAbilityIcon));
            const auto& iconTexture = textures_.at(currentAbilityId(version).value());
            if (!sf::FloatRect{iconPosition, sf::Vector2f{iconTexture.getSize()}}.contains(mousePosition_)) continue;

            const auto ability = abilityForVersion(version);
            const auto description = abilityDescription(ability);
            const auto primaryBounds = drawTooltip(description, iconPosition + sf::Vector2f{120.0F, -10.0F});
            const auto statusDetails = abilityStatusDetails(ability);
            if (!statusDetails.empty())
            {
                const auto referenceSize = sf::Vector2f{static_cast<float>(content_.presentation.referenceSize.x),
                                                        static_cast<float>(content_.presentation.referenceSize.y)};
                static_cast<void>(drawStatusGlossary(
                    window_,
                    interfaceFont_,
                    statusDetails,
                    [this](const auto& status) { return &textures_.at(status.value()); },
                    {referenceSize.x - 20.0F, 70.0F},
                    primaryBounds));
            }
            return;
        }
    }

    void SkillPanel::drawVersionSelectorTooltip() const
    {
        const auto activeVersion = state_.activeAbilityVersion(currentPlayerId(), abilityIndex_);
        for (std::size_t version = 0; version < 2; ++version)
        {
            if (!versionSelectorBounds(version).contains(mousePosition_)) continue;
            const auto description = version == activeVersion ? "Diese Fähigkeitsvariante ist aktiv."
                                                               : "Diese Fähigkeitsvariante ist inaktiv.\nKlicken, um sie zu aktivieren.";
            static_cast<void>(drawTooltip(description, mousePosition_ + sf::Vector2f{18.0F, -85.0F}));
            return;
        }
    }

    sf::FloatRect SkillPanel::drawTooltip(const std::string& description, sf::Vector2f preferredPosition) const
    {
        const auto layout = layoutFormattedText(interfaceFont_, description, 18U, 500.0F, 25.0F);
        const sf::Vector2f tooltipSize{std::max(80.0F, layout.size.x + 28.0F), layout.size.y + 24.0F};
        const auto tooltipPosition = clampedTooltipPosition(
            preferredPosition,
            tooltipSize,
            {static_cast<float>(content_.presentation.referenceSize.x), static_cast<float>(content_.presentation.referenceSize.y)});
        sf::RectangleShape background{tooltipSize};
        background.setPosition(tooltipPosition);
        background.setFillColor(sf::Color{20, 20, 20, 235});
        background.setOutlineColor(sf::Color{205, 175, 95});
        background.setOutlineThickness(2.0F);
        window_.draw(background);
        drawFormattedText(window_, interfaceFont_, layout, tooltipPosition + sf::Vector2f{14.0F, 12.0F});
        return {tooltipPosition, tooltipSize};
    }
} // namespace soma::presentation
