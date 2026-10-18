#include "presentation/sfml/SkillPanel.hpp"

#include "game/CampaignBattleContentBuilder.hpp"
#include "presentation/sfml/EconomyButton.hpp"
#include "presentation/sfml/FormattedTextLayout.hpp"
#include "presentation/sfml/StatusGlossary.hpp"
#include "presentation/sfml/UiCursor.hpp"
#include "presentation/text/AbilityDescription.hpp"
#include "presentation/text/SkillDescription.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

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

        content::Point add(content::Point left, content::Point right)
        {
            return {left.x + right.x, left.y + right.y};
        }
    } // namespace

    SkillPanel::SkillPanel(sf::RenderWindow& window,
                           const content::BattleContent& battleContent,
                           const content::EconomyContent& content,
                           game::CampaignState& state,
                           game::EconomyService& economy,
                           const EconomyTextures& textures,
                           const sf::Font& interfaceFont,
                           const sf::Font& titleFont,
                           const sf::Font& abilityFont,
                           const sf::Font& priceFont,
                           bool closeWindowOnExit,
                           std::function<void()> drawBackground,
                           std::function<bool(sf::Vector2f)> navigate,
                           std::function<bool(sf::Vector2f)> navigationHovered)
        : window_(window)
        , battleContent_(battleContent)
        , content_(content)
        , state_(state)
        , economy_(economy)
        , textures_(textures)
        , interfaceFont_(interfaceFont)
        , titleFont_(titleFont)
        , abilityFont_(abilityFont)
        , priceFont_(priceFont)
        , closeWindowOnExit_(closeWindowOnExit)
        , drawBackground_(std::move(drawBackground))
        , navigate_(std::move(navigate))
        , navigationHovered_(std::move(navigationHovered))
    {
        selectedVersion_ = state_.activeAbilityVersion(currentPlayerId(), abilityIndex_);
    }

    void SkillPanel::processEvents()
    {
        while (const auto event = window_.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window_.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>(); key && key->code == sf::Keyboard::Key::Escape) requestClose();
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>();
                click && click->button == sf::Mouse::Button::Left)
            {
                mousePressed_ = true;
                mousePosition_ = window_.mapPixelToCoords(click->position);
            }
            if (const auto* release = event->getIf<sf::Event::MouseButtonReleased>();
                release && release->button == sf::Mouse::Button::Left)
            {
                mousePressed_ = false;
                mousePosition_ = window_.mapPixelToCoords(release->position);
                if (navigate_ && navigate_(mousePosition_)) requestClose();
                else handleClick(mousePosition_);
            }
            if (const auto* movement = event->getIf<sf::Event::MouseMoved>())
                mousePosition_ = window_.mapPixelToCoords(movement->position);
        }
    }

    bool SkillPanel::closeRequested() const noexcept
    {
        return closeRequested_;
    }

    void SkillPanel::showPlayer(std::size_t index)
    {
        playerIndex_ = index % playerKeys_.size();
        abilityIndex_ = 0;
        selectedVersion_ = state_.activeAbilityVersion(currentPlayerId(), abilityIndex_);
        skillIndex_ = 0;
    }

    void SkillPanel::requestClose()
    {
        closeRequested_ = true;
        if (closeWindowOnExit_) window_.close();
    }

    void SkillPanel::render()
    {
        window_.clear(sf::Color{30, 30, 30});
        if (drawBackground_) drawBackground_();
        drawBackground();
        drawAbilityBridge();
        drawSkillGraph();
        drawPlayerAndAbilities();
        drawSkills();
        drawSelectionDetails();
        drawHoveredSkillTooltip();
        drawSelectedAbilityTooltip();
        drawVersionSelectorTooltip();
        setUiCursor(window_, clickableAt(mousePosition_) || (navigationHovered_ && navigationHovered_(mousePosition_)));
        window_.display();
    }

    gameplay::ContentDefinitionId SkillPanel::currentPlayerId() const
    {
        return gameplay::ContentDefinitionId{"player." + std::string{playerKeys_.at(playerIndex_)}};
    }

    const content::AbilitySetDefinition& SkillPanel::currentAbilitySet() const
    {
        const auto* set = game::EconomyQueries{state_, content_}.abilitySet(currentPlayerId(), abilityIndex_);
        if (set == nullptr) throw std::runtime_error("Missing ability set for the selected player and slot");
        return *set;
    }

    gameplay::AbilityId SkillPanel::currentAbilityId(std::size_t version) const
    {
        return currentAbilitySet().versions.at(version).ability;
    }

    std::vector<game::SkillEntry> SkillPanel::currentSkills(std::size_t version) const
    {
        return game::EconomyQueries{state_, content_}.skills(currentPlayerId(), abilityIndex_, version);
    }

    content::AbilityDefinition SkillPanel::abilityForVersion(std::size_t version) const
    {
        auto previewState = game::CampaignState::fromData(state_.data());
        game::EconomyService previewEconomy{previewState};
        static_cast<void>(previewEconomy.selectAbilityVersion(currentPlayerId(), abilityIndex_, version));
        const auto preview = game::CampaignBattleContentBuilder{}.build(battleContent_, content_, previewState);
        return preview.abilities.at(currentAbilityId(version));
    }

    sf::Vector2f SkillPanel::absolute(content::Point relative) const
    {
        const auto origin = content_.presentation.skills.origin;
        return {static_cast<float>(origin.x + relative.x), static_cast<float>(origin.y + relative.y)};
    }

    sf::FloatRect SkillPanel::absoluteBounds(content::Rectangle relative) const
    {
        return {absolute({relative.x, relative.y}), {static_cast<float>(relative.width), static_cast<float>(relative.height)}};
    }

    sf::FloatRect SkillPanel::boundsAt(std::string_view asset, content::Point relative) const
    {
        const auto& texture = textures_.at(asset);
        return {absolute(relative), {static_cast<float>(texture.getSize().x / 2U), static_cast<float>(texture.getSize().y)}};
    }

    sf::FloatRect SkillPanel::versionSelectorBounds(std::size_t version) const
    {
        const auto& layout = content_.presentation.skills;
        const auto position = absolute(add(layout.treeOrigins.at(version), layout.versionSelectors.at(version)));
        return {position, sf::Vector2f{textures_.at("skills.ability_active").getSize()}};
    }

    bool SkillPanel::canBuy(const game::SkillEntry& skill) const
    {
        return skill.availability == game::SkillAvailability::available && state_.currency(gameplay::Currency::dice) >= skill.cost;
    }

    bool SkillPanel::clickableAt(sf::Vector2f position) const
    {
        const auto& layout = content_.presentation.skills;
        if (boundsAt("skills.previous", layout.previousButton).contains(position) ||
            boundsAt("skills.next", layout.nextButton).contains(position) ||
            boundsAt("skills.close", layout.closeButton).contains(position))
            return true;
        for (const auto& box : layout.abilityHitboxes)
            if (sf::FloatRect{absolute({box.x, box.y}), {static_cast<float>(box.width), static_cast<float>(box.height)}}.contains(position))
                return true;
        for (std::size_t version = 0; version < 2; ++version)
        {
            const auto tree = layout.treeOrigins.at(version);
            const auto root = add(tree, layout.versionAbilityIcon);
            const auto& abilityTexture = textures_.at(currentAbilityId(version).value());
            if (sf::FloatRect{absolute(root), sf::Vector2f{abilityTexture.getSize()}}.contains(position) ||
                versionSelectorBounds(version).contains(position))
                return true;
            const auto skills = currentSkills(version);
            for (std::size_t index = 0; index < skills.size(); ++index)
            {
                if (sf::FloatRect{absolute(add(tree, layout.skillIcons.at(index))),
                                  sf::Vector2f{textures_.at(skills.at(index).icon).getSize()}}
                        .contains(position))
                    return true;
            }
        }
        const auto skills = currentSkills(selectedVersion_);
        return !skills.empty() && skills.at(std::min(skillIndex_, skills.size() - 1)).availability != game::SkillAvailability::acquired &&
               boundsAt("skills.buy", layout.buyButton).contains(position);
    }

    void SkillPanel::handleClick(sf::Vector2f position)
    {
        const auto& layout = content_.presentation.skills;
        if (boundsAt("skills.previous", layout.previousButton).contains(position))
        {
            sounds_.click();
            choosePreviousPlayer();
            return;
        }
        if (boundsAt("skills.next", layout.nextButton).contains(position))
        {
            sounds_.click();
            chooseNextPlayer();
            return;
        }
        if (boundsAt("skills.close", layout.closeButton).contains(position))
        {
            sounds_.click();
            requestClose();
            return;
        }
        for (std::size_t index = 0; index < layout.abilityHitboxes.size(); ++index)
        {
            const auto& box = layout.abilityHitboxes.at(index);
            if (sf::FloatRect{absolute({box.x, box.y}), {static_cast<float>(box.width), static_cast<float>(box.height)}}.contains(position))
            {
                abilityIndex_ = index;
                selectedVersion_ = state_.activeAbilityVersion(currentPlayerId(), abilityIndex_);
                skillIndex_ = 0;
                return;
            }
        }
        for (std::size_t version = 0; version < 2; ++version)
        {
            const auto tree = layout.treeOrigins.at(version);
            const auto root = add(tree, layout.versionAbilityIcon);
            const auto& abilityTexture = textures_.at(currentAbilityId(version).value());
            if (sf::FloatRect{absolute(root), sf::Vector2f{abilityTexture.getSize()}}.contains(position))
            {
                selectedVersion_ = version;
                skillIndex_ = 0;
                return;
            }
            if (versionSelectorBounds(version).contains(position))
            {
                sounds_.click();
                selectVersion(version);
                return;
            }
            const auto skills = currentSkills(version);
            for (std::size_t index = 0; index < skills.size(); ++index)
            {
                if (sf::FloatRect{absolute(add(tree, layout.skillIcons.at(index))),
                                  sf::Vector2f{textures_.at(skills.at(index).icon).getSize()}}
                        .contains(position))
                {
                    selectedVersion_ = version;
                    skillIndex_ = index;
                    return;
                }
            }
        }
        if (boundsAt("skills.buy", layout.buyButton).contains(position))
        {
            buySelectedSkill();
        }
    }

    void SkillPanel::selectVersion(std::size_t version)
    {
        selectedVersion_ = version;
        skillIndex_ = 0;
        static_cast<void>(economy_.selectAbilityVersion(currentPlayerId(), abilityIndex_, version));
    }

    void SkillPanel::choosePreviousPlayer()
    {
        playerIndex_ = playerIndex_ == 0 ? playerKeys_.size() - 1 : playerIndex_ - 1;
        abilityIndex_ = 0;
        selectedVersion_ = state_.activeAbilityVersion(currentPlayerId(), abilityIndex_);
        skillIndex_ = 0;
    }

    void SkillPanel::chooseNextPlayer()
    {
        playerIndex_ = (playerIndex_ + 1) % playerKeys_.size();
        abilityIndex_ = 0;
        selectedVersion_ = state_.activeAbilityVersion(currentPlayerId(), abilityIndex_);
        skillIndex_ = 0;
    }

    void SkillPanel::buySelectedSkill()
    {
        const auto skills = currentSkills(selectedVersion_);
        if (skillIndex_ >= skills.size() || !canBuy(skills.at(skillIndex_))) return;
        const auto& definition = content_.skills.at(skills.at(skillIndex_).skill);
        const auto result = economy_.acquireSkill(definition.id, definition.cost, definition.prerequisites);
        if (result.hasValue())
        {
            sounds_.skillAcquired();
        }
    }

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
            const auto root = add(layout.treeOrigins.at(version), layout.versionAbilityIcon);
            if (version == selectedVersion_)
            {
                sf::Sprite highlight{textures_.at("skills.acquired")};
                highlight.setPosition(absolute(add(root, layout.acquiredInset)));
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
            selector.setPosition(absolute(add(tree, layout.versionSelectors.at(version))));
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
                line.setPosition(absolute(add(tree, layout.connectionOffsets.at(connection))));
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
                const auto position = absolute(add(tree, layout.skillIcons.at(index)));
                if (skills.at(index).availability == game::SkillAvailability::acquired)
                {
                    sf::Sprite acquired{textures_.at("skills.acquired")};
                    acquired.setPosition(absolute(add(add(tree, layout.skillIcons.at(index)), layout.acquiredInset)));
                    window_.draw(acquired);
                }
                sf::Sprite icon{textures_.at(skills.at(index).icon)};
                icon.setPosition(position);
                if (skills.at(index).availability == game::SkillAvailability::locked) icon.setColor(sf::Color{40, 40, 40});
                window_.draw(icon);
            }
        }
        const auto position = add(content_.presentation.skills.treeOrigins.at(selectedVersion_),
                                  content_.presentation.skills.skillIcons.at(skillIndex_));
        sf::Sprite selection{textures_.at("skills.selection")};
        selection.setPosition(absolute(add(position, content_.presentation.skills.selectionInset)));
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
                const auto position = absolute(add(tree, layout.skillIcons.at(index)));
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
            const auto iconPosition = absolute(add(layout.treeOrigins.at(version), layout.versionAbilityIcon));
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
