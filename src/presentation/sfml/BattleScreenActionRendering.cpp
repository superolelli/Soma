#include "presentation/sfml/BattleHud.hpp"
#include "presentation/sfml/BattleScreenRenderer.hpp"
#include "presentation/sfml/NinePatch.hpp"

#include <ranges>
#include <sstream>
#include <unordered_map>

namespace
{
    sf::Text makeText(const sf::Font& font, const std::string& value, unsigned int size, sf::Vector2f position, sf::Color color = sf::Color::White)
    {
        sf::Text result{font, sf::String::fromUtf8(value.begin(), value.end()), size};
        result.setPosition(position);
        result.setFillColor(color);
        result.setOutlineColor(sf::Color::Black);
        result.setOutlineThickness(3.0F);
        return result;
    }

    sf::Sprite makeSprite(const sf::Texture& texture, sf::Vector2f position)
    {
        sf::Sprite result{texture};
        result.setPosition(position);
        return result;
    }
} // namespace

namespace soma::presentation
{
    void BattleScreenRenderer::drawAnnouncement()
    {
        if (!action_.has_value() || action_->stage != ActionStage::announcement)
        {
            return;
        }
        const auto& layout = content_.layout;
        const auto& ability = content_.abilities.at(action_->command.ability);
        const auto bannerPosition = sf::Vector2f{static_cast<float>(layout.announcementX), static_cast<float>(layout.announcementY)};
        window_.draw(makeSprite(textures_.announcement, bannerPosition));
        auto label = makeText(interfaceFont_, ability.name, 25U, {});
        const auto labelBounds = label.getLocalBounds();
        const auto bannerSize =
            sf::Vector2f{static_cast<float>(textures_.announcement.getSize().x), static_cast<float>(textures_.announcement.getSize().y)};
        label.setPosition({bannerPosition.x + (bannerSize.x - labelBounds.size.x) / 2.0F - labelBounds.position.x,
                           bannerPosition.y + (bannerSize.y - labelBounds.size.y) / 2.0F - labelBounds.position.y});
        window_.draw(label);
        for (const auto& target : action_->affectedCombatants)
        {
            drawTargetMarker(target);
        }
    }

    void BattleScreenRenderer::drawItemInterruption()
    {
        if (!action_.has_value() || action_->stage != ActionStage::itemInterruption)
        {
            return;
        }
        const auto activation = std::ranges::find_if(action_->events, [](const auto& event) {
            return std::holds_alternative<gameplay::ItemActivated>(event);
        });
        if (activation == action_->events.end())
        {
            return;
        }
        const auto& item = std::get<gameplay::ItemActivated>(*activation);
        const auto presentation = content_.itemActivations.find(item.item);
        const auto speaker = views_.find(item.carrier);
        if (presentation == content_.itemActivations.end() || speaker == views_.end())
        {
            return;
        }

        const auto& layout = content_.layout;
        sf::Text text{interfaceFont_, "", static_cast<unsigned int>(layout.itemSpeechCharacterSize)};
        text.setFillColor(sf::Color::Black);
        std::istringstream words{presentation->second.speech};
        std::string word;
        std::string line;
        std::string wrapped;
        while (words >> word)
        {
            const auto candidate = line.empty() ? word : line + " " + word;
            text.setString(sf::String::fromUtf8(candidate.begin(), candidate.end()));
            if (!line.empty() && text.getLocalBounds().size.x > static_cast<float>(layout.itemSpeechMaximumWidth))
            {
                wrapped += (wrapped.empty() ? "" : "\n") + line;
                line = word;
            }
            else
            {
                line = candidate;
            }
        }
        wrapped += (wrapped.empty() ? "" : "\n") + line;
        text.setString(sf::String::fromUtf8(wrapped.begin(), wrapped.end()));

        const auto textBounds = text.getLocalBounds();
        NinePatch background{textures_.itemSpeechBubble};
        background.setSize(
            {textBounds.size.x + static_cast<float>(layout.itemSpeechPaddingX), textBounds.size.y + static_cast<float>(layout.itemSpeechPaddingY)});
        const auto bubbleBounds = background.globalBounds();
        const auto& speakerBounds = speaker->second.bounds;
        const sf::Vector2f position{speakerBounds.position.x + speakerBounds.size.x + static_cast<float>(layout.itemSpeechSpeakerOffsetX),
                                    speakerBounds.position.y - bubbleBounds.size.y + static_cast<float>(layout.itemSpeechSpeakerOffsetY)};
        background.setPosition(position);
        text.setPosition({position.x + (bubbleBounds.size.x - textBounds.size.x) / 2.0F - textBounds.position.x,
                          position.y + (bubbleBounds.size.y - textBounds.size.y) / 2.0F - textBounds.position.y +
                              static_cast<float>(layout.itemSpeechTextVerticalOffset)});
        window_.draw(background);
        window_.draw(text);
    }

    void BattleScreenRenderer::drawInformationPanel()
    {
        auto informationCombatant = interaction_.inspectedCombatant();
        if (!informationCombatant.has_value() && !battle_.finished())
        {
            informationCombatant = battle_.currentActor();
        }
        if (!informationCombatant.has_value())
        {
            return;
        }
        const auto* state = battle_.find(*informationCombatant);
        if (state == nullptr)
        {
            return;
        }

        const auto displayed = displayedState(*state);
        drawCombatantInformation(
            window_, textures_, interfaceFont_, displayed, content_.combatants.at(state->definition), content_.layout, views_.at(state->id));
        const auto& selectedAbility = interaction_.selectedAbility();
        if (!selectedAbility.has_value() || battle_.finished())
        {
            return;
        }
        const auto* actor = battle_.find(battle_.currentActor());
        const auto validTargets = battle_.validPrimaryTargets(*selectedAbility);
        if (actor != nullptr && std::ranges::find(validTargets, state->id) != validTargets.end())
        {
            drawExpectedAttack(window_, interfaceFont_, battle_, *actor, *state, content_.abilities.at(*selectedAbility), content_.layout);
        }
    }

    void BattleScreenRenderer::drawFloatingTexts()
    {
        std::unordered_map<gameplay::EntityId, int> rowsByCombatant;
        for (const auto& floating : floatingTexts_)
        {
            const auto& font = floating.font == FloatingTextFont::interface ? interfaceFont_ : notificationFont_;
            auto notification = makeText(font, floating.value, floating.size, {0.0F, 0.0F}, floating.color);
            notification.setOutlineColor(floating.outlineColor);
            notification.setOutlineThickness(floating.outlineThickness);
            const auto textBounds = notification.getLocalBounds();

            const auto found = views_.find(floating.subject);
            if (found == views_.end())
            {
                continue;
            }
            const auto& bounds = found->second.bounds;
            const auto row = rowsByCombatant[floating.subject]++;
            notification.setPosition({bounds.position.x + bounds.size.x / 2.0F - textBounds.size.x / 2.0F - textBounds.position.x,
                                      bounds.position.y - floating.elapsed * 120.0F - row * 45.0F});
            window_.draw(notification);
        }
    }

    void BattleScreenRenderer::drawBattleResult()
    {
        if (!battle_.finished() || action_.has_value())
        {
            return;
        }
        const auto message = battle_.winner() == gameplay::Team::players ? "Sieg!" : "Niederlage";
        window_.draw(makeText(
            interfaceFont_, message, 70U, {static_cast<float>(content_.layout.battleResultX), static_cast<float>(content_.layout.battleResultY)}));
    }

    void BattleScreenRenderer::drawTargetMarker(const gameplay::EntityId& target)
    {
        const auto& view = views_.at(target);
        const auto x = view.bounds.position.x + view.bounds.size.x / 2.0F - textures_.targetMarker.getSize().x / 2.0F;
        window_.draw(makeSprite(textures_.targetMarker, {x, view.bounds.position.y + view.bounds.size.y + 49.0F}));
    }

} // namespace soma::presentation
