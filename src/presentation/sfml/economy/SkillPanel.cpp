#include "presentation/sfml/economy/SkillPanel.hpp"

#include "game/CampaignBattleContentBuilder.hpp"
#include "presentation/sfml/economy/EconomyButton.hpp"
#include "presentation/sfml/common/FormattedTextLayout.hpp"
#include "presentation/sfml/economy/SkillPanelLayout.hpp"
#include "presentation/sfml/common/StatusGlossary.hpp"
#include "presentation/sfml/common/UiCursor.hpp"
#include "presentation/text/AbilityDescription.hpp"
#include "presentation/text/SkillDescription.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace soma::presentation
{
    SkillPanel::SkillPanel(sf::RenderWindow& window,
                           const content::BattleContent& battleContent,
                           const content::EconomyContent& content,
                           game::CampaignState& state,
                           game::EconomyService& economy,
                           const EconomyTextures& textures,
                           const EconomyPanelFonts& fonts,
                           PanelHostOptions host)
        : window_(window)
        , battleContent_(battleContent)
        , content_(content)
        , state_(state)
        , economy_(economy)
        , textures_(textures)
        , interfaceFont_(fonts.interface)
        , titleFont_(fonts.title)
        , abilityFont_(fonts.ability)
        , priceFont_(fonts.price)
        , closeWindowOnExit_(host.closeWindowOnExit)
        , drawBackground_(std::move(host.drawBackground))
        , navigate_(std::move(host.navigate))
        , navigationHovered_(std::move(host.navigationHovered))
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
        const auto position = absolute(detail::addPoints(layout.treeOrigins.at(version), layout.versionSelectors.at(version)));
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
            const auto root = detail::addPoints(tree, layout.versionAbilityIcon);
            const auto& abilityTexture = textures_.at(currentAbilityId(version).value());
            if (sf::FloatRect{absolute(root), sf::Vector2f{abilityTexture.getSize()}}.contains(position) ||
                versionSelectorBounds(version).contains(position))
                return true;
            const auto skills = currentSkills(version);
            for (std::size_t index = 0; index < skills.size(); ++index)
            {
                if (sf::FloatRect{absolute(detail::addPoints(tree, layout.skillIcons.at(index))),
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
            const auto root = detail::addPoints(tree, layout.versionAbilityIcon);
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
                if (sf::FloatRect{absolute(detail::addPoints(tree, layout.skillIcons.at(index))),
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

} // namespace soma::presentation
