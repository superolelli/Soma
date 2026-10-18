#include "presentation/sfml/BattleHud.hpp"

#include "presentation/sfml/FormattedTextLayout.hpp"
#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <ranges>

namespace soma::presentation
{
    using simulation::CombatantState;

    namespace
    {
        sf::Text text(const sf::Font& font, const std::string& value, unsigned int size, sf::Vector2f position, sf::Color color = sf::Color::White)
        {
            sf::Text result{font, sf::String::fromUtf8(value.begin(), value.end()), size};
            result.setPosition(position);
            result.setFillColor(color);
            result.setOutlineColor(sf::Color::Black);
            result.setOutlineThickness(3.0F);
            return result;
        }

        sf::Vector2f sizeOf(const sf::Texture& texture)
        {
            return {static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y)};
        }

        sf::Sprite sprite(const sf::Texture& texture, sf::Vector2f position)
        {
            sf::Sprite result{texture};
            result.setPosition(position);
            return result;
        }
    } // namespace

    void drawHealthBar(sf::RenderWindow& window, const BattleTextures& textures, const CombatantView& view)
    {
        const auto x = view.bounds.position.x + view.bounds.size.x / 2.0F - textures.healthBarFrame.getSize().x / 2.0F;
        const auto y = view.bounds.position.y + view.bounds.size.y + 30.0F;
        const auto ratio = std::clamp(static_cast<float>(view.displayedHealth) / static_cast<float>(view.maximumHealth), 0.0F, 1.0F);
        auto fill = sprite(textures.healthBar, {x, y});
        const auto fillWidth = textures.healthBar.getSize().x / 2U;
        fill.setTextureRect({{0, 0}, {static_cast<int>(fillWidth * ratio), static_cast<int>(textures.healthBar.getSize().y)}});
        window.draw(fill);
        window.draw(sprite(textures.healthBarFrame, {x, y}));
    }

    sf::Vector2f fatigueBarPosition(const BattleTextures& textures, const CombatantView& view)
    {
        const auto width = static_cast<float>(textures.fatigueBarFrame.getSize().x);
        const auto healthY = view.bounds.position.y + view.bounds.size.y + 30.0F;
        return {view.bounds.position.x + view.bounds.size.x / 2.0F - width / 2.0F,
                healthY + static_cast<float>(textures.healthBarFrame.getSize().y) + 2.0F};
    }

    void drawFatigueBar(sf::RenderWindow& window, const BattleTextures& textures, const CombatantState& state, const CombatantView& view)
    {
        if (state.team != gameplay::Team::players)
        {
            return;
        }
        const auto position = fatigueBarPosition(textures, view);
        const auto fillWidth = textures.fatigueBar.getSize().x / 2U;
        const auto ratio = std::clamp(view.displayedFatigue / static_cast<float>(gameplay::maximumFatigue), 0.0F, 1.0F);
        const auto visibleWidth = static_cast<unsigned int>(static_cast<float>(fillWidth) * ratio);
        if (visibleWidth > 0U)
        {
            auto fill = sprite(textures.fatigueBar, position);
            fill.setTextureRect({{0, 0}, {static_cast<int>(visibleWidth), static_cast<int>(textures.fatigueBar.getSize().y)}});
            window.draw(fill);
        }
        window.draw(sprite(textures.fatigueBarFrame, position));
    }

    void drawCombatantShadow(sf::RenderWindow& window, const CombatantView& view)
    {
        const auto fallback = AnimationPoint{view.bounds.position.x + view.bounds.size.x / 2.0F,
                                             view.bounds.position.y + view.bounds.size.y};
        const auto shadowPoint = view.animation->point("shadow_point").value_or(fallback);
        sf::CircleShape shadow{view.bounds.size.x / 2.0F + 10.0F, 60U};
        shadow.setScale({1.0F, 0.2F});
        shadow.setFillColor(sf::Color{0, 0, 0, 100});
        const auto bounds = shadow.getGlobalBounds();
        shadow.setPosition({shadowPoint.x - bounds.size.x / 2.0F, shadowPoint.y - bounds.size.y / 2.0F});
        window.draw(shadow);
    }

    void appendAttribute(std::string& tooltip, int value, const std::string& name)
    {
        if (value == 0)
        {
            return;
        }
        tooltip += "\n#aaaadd ";
        tooltip += value > 0 ? "+" : "";
        tooltip += std::to_string(value) + " " + name;
    }

    void appendPercentage(std::string& tooltip, double value, const std::string& name)
    {
        if (value == 0.0)
        {
            return;
        }
        tooltip += "\n#aaaadd ";
        tooltip += value > 0.0 ? "+" : "";
        tooltip += std::to_string(static_cast<int>(std::round(value * 100.0))) + "% " + name;
    }

    std::string buffTooltip(const soma::simulation::ActiveBuff& buff)
    {
        std::string result;
        appendAttribute(result, buff.attributes.armour, "Rüstung");
        if (buff.attributes.damageMinimum == buff.attributes.damageMaximum)
        {
            appendAttribute(result, buff.attributes.damageMinimum, "Schaden");
        }
        else
        {
            appendAttribute(result, buff.attributes.damageMinimum, "minimaler Schaden");
            appendAttribute(result, buff.attributes.damageMaximum, "maximaler Schaden");
        }
        appendAttribute(result, buff.attributes.initiative, "Initiative");
        appendAttribute(result, buff.attributes.maximumHealth, "maximale Gesundheit");
        appendAttribute(result, buff.attributes.criticalHitChance, "kritische Trefferchance");
        appendAttribute(result, buff.attributes.precision, "Präzision");
        appendAttribute(result, buff.attributes.dodge, "Ausweichen");
        appendAttribute(result, buff.attributes.healing, "Heilung");
        appendAttribute(result, buff.attributes.sleepResistance, "Schlafresistenz");
        appendAttribute(result, buff.attributes.confusionResistance, "Verwirrungsresistenz");
        appendAttribute(result, buff.attributes.debuffResistance, "Debuffresistenz");
        appendAttribute(result, buff.attributes.decayResistance, "Verfallsresistenz");
        appendPercentage(result, buff.healingReceived, "erhaltene Heilung");
        appendPercentage(result, buff.damageAgainstMarked, "Schaden gegen Markierte");
        appendPercentage(result,
                         buff.damagePerNegativeStatus * buff.negativeStatusCountAtApplication,
                         "verursachter Schaden");
        if (buff.damageFromFatigue)
        {
            result += "\n#aaaadd Direktschaden: +relative Müdigkeit";
        }
        if (buff.blocksFatigueStageOne)
        {
            result += "\n#aaaadd Blockiert den Müdigkeitsmalus bei 9";
        }
        if (buff.blocksFatigueStageTwo)
        {
            result += "\n#aaaadd Blockiert den Müdigkeitsmalus bei 18";
        }
        return result;
    }

    void mergeBuffTooltipEffect(simulation::ActiveBuff& destination, const simulation::ActiveBuff& source)
    {
        destination.attributes.armour += source.attributes.armour;
        destination.attributes.damageMinimum += source.attributes.damageMinimum;
        destination.attributes.damageMaximum += source.attributes.damageMaximum;
        destination.attributes.initiative += source.attributes.initiative;
        destination.attributes.maximumHealth += source.attributes.maximumHealth;
        destination.attributes.criticalHitChance += source.attributes.criticalHitChance;
        destination.attributes.precision += source.attributes.precision;
        destination.attributes.dodge += source.attributes.dodge;
        destination.attributes.healing += source.attributes.healing;
        destination.attributes.sleepResistance += source.attributes.sleepResistance;
        destination.attributes.confusionResistance += source.attributes.confusionResistance;
        destination.attributes.debuffResistance += source.attributes.debuffResistance;
        destination.attributes.decayResistance += source.attributes.decayResistance;
        destination.healingReceived += source.healingReceived;
        destination.damageAgainstMarked += source.damageAgainstMarked;
        destination.damagePerNegativeStatus += source.damagePerNegativeStatus * source.negativeStatusCountAtApplication;
        destination.negativeStatusCountAtApplication = 1;
        destination.damageFromFatigue = destination.damageFromFatigue || source.damageFromFatigue;
        destination.blocksFatigueStageOne = destination.blocksFatigueStageOne || source.blocksFatigueStageOne;
        destination.blocksFatigueStageTwo = destination.blocksFatigueStageTwo || source.blocksFatigueStageTwo;
    }

    std::string combinedBuffTooltip(const std::vector<simulation::ActiveBuff>& effects)
    {
        struct TooltipEffect
        {
            int duration{};
            simulation::ActiveBuff combined;
        };

        std::vector<TooltipEffect> groupedEffects;
        for (const auto& effect : effects)
        {
            const auto duration = std::max(0, effect.remainingTurns);
            const auto existing = std::ranges::find(groupedEffects, duration, &TooltipEffect::duration);
            if (existing == groupedEffects.end())
            {
                groupedEffects.push_back({duration, {}});
                groupedEffects.back().combined.remainingTurns = duration;
                mergeBuffTooltipEffect(groupedEffects.back().combined, effect);
            }
            else
            {
                mergeBuffTooltipEffect(existing->combined, effect);
            }
        }

        std::string result;
        for (std::size_t index = 0; index < groupedEffects.size(); ++index)
        {
            const auto& effect = groupedEffects.at(index);
            if (!result.empty())
            {
                result += "\n\n";
            }
            result += "#d6b45c Effekt " + std::to_string(index + 1) + "#ffffff (" + std::to_string(effect.duration) +
                      (effect.duration == 1 ? " Runde)" : " Runden)") + buffTooltip(effect.combined);
        }
        return result;
    }

    std::string decayTooltip(const std::vector<simulation::ActiveDecay>& effects)
    {
        std::string result = "Verfall";
        for (const auto& effect : effects)
        {
            const auto duration = std::max(0, effect.remainingTurns);
            result += "\n" + std::to_string(std::max(0, effect.damage)) + " Schaden für noch " + std::to_string(duration) +
                      (duration == 1 ? " Runde" : " Runden");
        }
        return result;
    }

    std::string transfusionTooltip(const std::vector<simulation::ActiveTransfusion>& effects)
    {
        std::string result = effects.size() == 1 ? "Transfusion" : std::to_string(effects.size()) + " Transfusionen";
        for (const auto& effect : effects)
        {
            result += "\nBei Verfallsschaden: Heilt einen zufälligen Gegner um " + std::to_string(std::max(0, effect.healing));
        }
        result += "\nEndet, sobald kein Verfall mehr vorhanden ist";
        return result;
    }

    std::vector<StatusIconInfo> activeStatusIcons(const BattleTextures& textures, const CombatantState& state)
    {
        std::vector<StatusIconInfo> icons;
        if (state.dynamite)
        {
            icons.push_back({&textures.dynamite, "Dynamit\n20% Explosion: 15 Schaden\nSonst Weitergabe", gameplay::StatusId{"status.dynamite"}});
        }
        if (state.sleeping)
        {
            icons.push_back({&textures.sleeping, "Schläft und überspringt den nächsten Zug", gameplay::StatusId{"status.sleeping"}});
        }
        if (state.fistOfRevengeTarget)
        {
            icons.push_back({&textures.fistOfRevenge, "Angreifbar durch Fist of Revenge", gameplay::StatusId{"status.fist_of_revenge"}});
        }
        if (state.nofaceBuffLevel >= 0)
        {
            icons.push_back({&textures.noface,
                             "Seine Wunden machen ihn stärker\nStufe " + std::to_string(state.nofaceBuffLevel),
                             gameplay::StatusId{"status.noface"}});
        }
        if (!state.decay.empty())
        {
            icons.push_back({&textures.decay, decayTooltip(state.decay), gameplay::StatusId{"status.decay"}});
        }
        if (state.confusedTurns > 0)
        {
            icons.push_back(
                {&textures.confused, "Verwirrt (" + std::to_string(state.confusedTurns) + " Runden)", gameplay::StatusId{"status.confused"}});
        }
        if (state.hackedTurns > 0)
        {
            icons.push_back({&textures.hacked,
                             "Gehackt (" + std::to_string(state.hackedTurns) + " Runden)\nKann keine Stärkungen erhalten",
                             gameplay::StatusId{"status.hacked"}});
        }
        if (!state.transfusions.empty())
        {
            icons.push_back({&textures.transfusion, transfusionTooltip(state.transfusions), gameplay::StatusId{"status.transfusion"}});
        }
        if (state.panicStacks > 0)
        {
            icons.push_back({&textures.panic,
                             "Panik (" + std::to_string(state.panicStacks) + " Stapel)\nPräzision: -" +
                                 std::to_string(state.panicStacks * 5),
                             gameplay::StatusId{"status.panic"}});
        }
        if (!state.buffs.empty())
        {
            icons.push_back({&textures.buff, combinedBuffTooltip(state.buffs), gameplay::StatusId{"status.buffed"}});
        }
        if (!state.debuffs.empty())
        {
            icons.push_back({&textures.debuff, combinedBuffTooltip(state.debuffs), gameplay::StatusId{"status.debuffed"}});
        }
        if (state.markedTurns > 0)
        {
            icons.push_back({&textures.marked, "Markiert (" + std::to_string(state.markedTurns) + " Runden)", gameplay::StatusId{"status.marked"}});
        }
        if (state.misses > 0)
        {
            icons.push_back({&textures.missed,
                             std::to_string(state.misses) + (state.misses == 1 ? " Fehlschuss" : " Fehlschüsse"),
                             gameplay::StatusId{"status.miss"}});
        }
        if (state.bountyTurns > 0)
        {
            icons.push_back({&textures.bounty,
                             "Kopfgeld (" + std::to_string(state.bountyTurns) + " Runden)\nBei Schaden: +2 Schaden für Angreifer",
                             gameplay::StatusId{"status.bounty"}});
        }
        if (state.riposteTurns > 0)
        {
            icons.push_back({&textures.riposte,
                             "Gegenschlag (" + std::to_string(state.riposteTurns) + " Runden)\nKontert jeden feindlichen Angriff",
                             gameplay::StatusId{"status.riposte"}});
        }
        if (state.rageTurns > 0)
        {
            icons.push_back({&textures.rage,
                             "Raserei (" + std::to_string(state.rageTurns) + " Runden)\nVerursachter und erlittener Schaden: +50%",
                             gameplay::StatusId{"status.rage"}});
        }
        const auto fatigueStage = gameplay::fatigueStage(state.fatigue);
        if (fatigueStage > 0)
        {
            const auto appliedStages = std::popcount(static_cast<unsigned int>(state.appliedFatiguePenaltyStages));
            const auto damagePenalty = appliedStages * 3;
            const auto generalPenalty = appliedStages * 5;
            const auto blocked = appliedStages == 0         ? "\n#88cc88 Dieser Effekt ist blockiert"
                                 : appliedStages < fatigueStage ? "\n#88cc88 Dieser Effekt ist teilweise blockiert"
                                                               : "";
            icons.push_back({fatigueStage == 1 ? &textures.fatigueTired : &textures.fatigueStupid,
                             "Müdigkeit Stufe " + std::to_string(fatigueStage) + "\n-" + std::to_string(damagePenalty) +
                                 " Schaden"
                                 "\n-" +
                                 std::to_string(generalPenalty) +
                                 " Ausweichen und Präzision"
                                 "\n-" +
                                 std::to_string(appliedStages) + " Initiative" + blocked,
                             gameplay::StatusId{"status.fatigue." + std::to_string(fatigueStage)}});
        }
        return icons;
    }

    void drawStatusIcons(sf::RenderWindow& window,
                         const BattleTextures& textures,
                         const CombatantState& state,
                         const CombatantView& view,
                         const std::vector<gameplay::StatusId>& newlyGainedStatuses,
                         float statusAnimationElapsed)
    {
        const auto healthBarX = view.bounds.position.x + view.bounds.size.x / 2.0F - textures.healthBarFrame.getSize().x / 2.0F;
        auto statusY = view.bounds.position.y + view.bounds.size.y + 30.0F + textures.healthBarFrame.getSize().y + 2.0F;
        if (state.team == gameplay::Team::players)
        {
            statusY += static_cast<float>(textures.fatigueBarFrame.getSize().y) + 2.0F;
        }
        auto position = sf::Vector2f{healthBarX, statusY};
        for (const auto& status : activeStatusIcons(textures, state))
        {
            auto icon = sprite(*status.texture, position);
            if (std::ranges::find(newlyGainedStatuses, status.status) != newlyGainedStatuses.end() && statusAnimationElapsed < 0.5F)
            {
                const auto progress = std::clamp(statusAnimationElapsed / 0.5F, 0.0F, 1.0F);
                const auto inverse = 1.0F - progress;
                const auto scale = inverse * inverse * inverse * 1.5F + 3.0F * progress * inverse * inverse * 0.64F +
                                   3.0F * progress * progress * inverse * 0.635F + progress * progress * progress;
                const auto iconSize = sizeOf(*status.texture);
                icon.setOrigin(iconSize / 2.0F);
                icon.setPosition(position + iconSize / 2.0F);
                icon.setScale({scale, scale});
            }
            window.draw(icon);
            position.x += 20.0F;
        }
    }

    sf::Vector2f tooltipSize(const sf::Font& font, const std::string& value, float maximumContentWidth)
    {
        const auto layout = layoutFormattedText(font, value, 16U, maximumContentWidth, 23.0F);
        return {layout.size.x + 24.0F, layout.size.y + 16.0F};
    }

    sf::FloatRect drawTooltip(sf::RenderWindow& window,
                              const sf::Font& font,
                              const std::string& value,
                              sf::Vector2f position,
                              bool alignBottom,
                              float maximumContentWidth)
    {
        const auto layout = layoutFormattedText(font, value, 16U, maximumContentWidth, 23.0F);
        const auto size = sf::Vector2f{layout.size.x + 24.0F, layout.size.y + 16.0F};
        if (alignBottom)
        {
            position.y -= size.y;
        }
        const auto view = window.getView();
        const auto viewOrigin = view.getCenter() - view.getSize() / 2.0F;
        const auto maximumX = std::max(viewOrigin.x + 5.0F, viewOrigin.x + view.getSize().x - size.x - 5.0F);
        const auto maximumY = std::max(viewOrigin.y + 5.0F, viewOrigin.y + view.getSize().y - size.y - 5.0F);
        position.x = std::clamp(position.x, viewOrigin.x + 5.0F, maximumX);
        position.y = std::clamp(position.y, viewOrigin.y + 5.0F, maximumY);
        sf::RectangleShape background{size};
        background.setPosition(position);
        background.setFillColor(sf::Color{0, 0, 0, 230});
        background.setOutlineColor(sf::Color{70, 70, 70});
        background.setOutlineThickness(2.0F);
        window.draw(background);

        drawFormattedText(window, font, layout, position + sf::Vector2f{12.0F, 8.0F});
        return {position, size};
    }

    sf::Color changedValueColor(int current, int base)
    {
        if (current > base)
        {
            return sf::Color::Green;
        }
        if (current < base)
        {
            return sf::Color::Red;
        }
        return sf::Color{220, 220, 220};
    }

    void drawLargeFatigueBar(sf::RenderWindow& window,
                             const BattleTextures& textures,
                             const sf::Font& font,
                             const CombatantState& state,
                             const CombatantView& view,
                             sf::Vector2f healthPosition)
    {
        if (state.team != gameplay::Team::players)
        {
            return;
        }

        const auto healthFrameSize = sizeOf(textures.healthBarBigFrame);
        const auto fatigueFrameSize = sizeOf(textures.fatigueBarBigFrame);
        const auto fatiguePosition =
            healthPosition + sf::Vector2f{(healthFrameSize.x - fatigueFrameSize.x) / 2.0F, healthFrameSize.y + 4.0F};
        const auto fillWidth = textures.fatigueBarBig.getSize().x / 2U;
        const auto ratio = std::clamp(view.displayedFatigue / static_cast<float>(gameplay::maximumFatigue), 0.0F, 1.0F);
        const auto visibleWidth = static_cast<unsigned int>(static_cast<float>(fillWidth) * ratio);
        if (visibleWidth > 0U)
        {
            const auto fillSize = sf::Vector2f{static_cast<float>(fillWidth), static_cast<float>(textures.fatigueBarBig.getSize().y)};
            auto fill = sprite(textures.fatigueBarBig,
                               fatiguePosition + sf::Vector2f{(fatigueFrameSize.x - fillSize.x) / 2.0F,
                                                              (fatigueFrameSize.y - fillSize.y) / 2.0F});
            fill.setTextureRect({{0, 0}, {static_cast<int>(visibleWidth), static_cast<int>(textures.fatigueBarBig.getSize().y)}});
            window.draw(fill);
        }
        window.draw(sprite(textures.fatigueBarBigFrame, fatiguePosition));

        const auto fatigueText = std::to_string(static_cast<int>(std::round(view.displayedFatigue))) + "/" +
                                 std::to_string(gameplay::maximumFatigue);
        auto fatigueLabel = text(font, fatigueText, 14U, {});
        const auto labelBounds = fatigueLabel.getLocalBounds();
        fatigueLabel.setPosition({fatiguePosition.x + (fatigueFrameSize.x - labelBounds.size.x) / 2.0F - labelBounds.position.x,
                                  fatiguePosition.y + (fatigueFrameSize.y - labelBounds.size.y) / 2.0F - labelBounds.position.y});
        window.draw(fatigueLabel);
    }

    void drawCombatantInformation(sf::RenderWindow& window,
                                  const BattleTextures& textures,
                                  const sf::Font& font,
                                  const CombatantState& state,
                                  const soma::content::CombatantDefinition& baseDefinition,
                                  const soma::content::BattleLayoutDefinition& layout,
                                  const CombatantView& view)
    {
        const sf::Vector2f panelPosition{static_cast<float>(layout.informationPanelX), static_cast<float>(layout.informationPanelY)};
        const auto healthPosition = panelPosition + sf::Vector2f{40.0F, 70.0F};
        window.draw(sprite(textures.combatantInformationPanel, panelPosition));

        auto name = text(font, state.name, 25U, {});
        const auto nameWidth = name.getLocalBounds().size.x;
        name.setPosition({healthPosition.x + (textures.healthBarBigFrame.getSize().x - nameWidth) / 2.0F, panelPosition.y + 25.0F});
        window.draw(name);

        const auto ratio = std::clamp(static_cast<float>(view.displayedHealth) / static_cast<float>(std::max(1, view.maximumHealth)), 0.0F, 1.0F);
        auto healthFill = sprite(textures.healthBarBig, healthPosition + sf::Vector2f{17.0F, 20.0F});
        const auto healthFrameWidth = textures.healthBarBig.getSize().x / 2U;
        healthFill.setTextureRect({{0, 0}, {static_cast<int>(healthFrameWidth * ratio), static_cast<int>(textures.healthBarBig.getSize().y)}});
        window.draw(healthFill);
        window.draw(sprite(textures.healthBarBigFrame, healthPosition));

        const auto healthText = std::to_string(static_cast<int>(view.displayedHealth)) + "/" + std::to_string(view.maximumHealth);
        auto healthLabel = text(font, healthText, 14U, {});
        const auto healthBounds = healthLabel.getLocalBounds();
        const auto healthFrameSize = sizeOf(textures.healthBarBigFrame);
        healthLabel.setPosition({healthPosition.x + (healthFrameSize.x - healthBounds.size.x) / 2.0F - healthBounds.position.x,
                                 healthPosition.y + (healthFrameSize.y - healthBounds.size.y) / 2.0F - healthBounds.position.y});
        window.draw(healthLabel);
        drawLargeFatigueBar(window, textures, font, state, view, healthPosition);

        const std::array attributeLabels{"Rüstung:", "Schaden:", "Initiative:", "Kritische Trefferchance:", "Ausweichen:", "Präzision:"};
        const auto nonNegative = [](int value) {
            return std::max(0, value);
        };
        const std::array attributeValues{
            std::to_string(nonNegative(state.attributes.armour)),
            std::to_string(nonNegative(state.attributes.damageMinimum)) + "-" + std::to_string(nonNegative(state.attributes.damageMaximum)),
            std::to_string(nonNegative(state.attributes.initiative)),
            std::to_string(nonNegative(state.attributes.criticalHitChance)),
            std::to_string(nonNegative(state.attributes.dodge)),
            std::to_string(nonNegative(state.attributes.precision))};
        const std::array attributeColors{changedValueColor(state.attributes.armour, baseDefinition.attributes.armour),
                                         changedValueColor(state.attributes.damageMinimum, baseDefinition.attributes.damageMinimum),
                                         changedValueColor(state.attributes.initiative, baseDefinition.attributes.initiative),
                                         changedValueColor(state.attributes.criticalHitChance, baseDefinition.attributes.criticalHitChance),
                                         changedValueColor(state.attributes.dodge, baseDefinition.attributes.dodge),
                                         changedValueColor(state.attributes.precision, baseDefinition.attributes.precision)};
        sf::RectangleShape attributeBackground{{390.0F, 145.0F}};
        attributeBackground.setPosition(panelPosition + sf::Vector2f{648.0F, 16.0F});
        attributeBackground.setFillColor(sf::Color{0, 0, 0, 220});
        attributeBackground.setOutlineColor(sf::Color{40, 40, 40});
        attributeBackground.setOutlineThickness(2.0F);
        window.draw(attributeBackground);
        auto y = panelPosition.y + 25.0F;
        for (std::size_t index = 0; index < attributeLabels.size(); ++index)
        {
            window.draw(text(font, attributeLabels.at(index), 16U, {panelPosition.x + 660.0F, y}, sf::Color{150, 150, 150}));
            window.draw(text(font, attributeValues.at(index), 16U, {panelPosition.x + 895.0F, y}, attributeColors.at(index)));
            y += 20.0F;
        }

        window.draw(text(font, "Resistenzen", 20U, panelPosition + sf::Vector2f{510.0F, 25.0F}));
        const std::array resistanceValues{nonNegative(state.secondaryAttributes.sleepResistance),
                                          nonNegative(state.secondaryAttributes.confusionResistance),
                                          nonNegative(state.secondaryAttributes.debuffResistance),
                                          nonNegative(state.secondaryAttributes.decayResistance)};
        const std::array<const sf::Texture*, 4> resistanceIcons{&textures.sleeping, &textures.confused, &textures.debuff, &textures.decay};
        y = panelPosition.y + 60.0F;
        for (std::size_t index = 0; index < resistanceValues.size(); ++index)
        {
            window.draw(sprite(*resistanceIcons.at(index), {panelPosition.x + 540.0F, y - 2.0F}));
            window.draw(text(font, std::to_string(resistanceValues.at(index)) + "%", 16U, {panelPosition.x + 570.0F, y}, sf::Color{220, 150, 0}));
            y += 25.0F;
        }
    }

    void drawExpectedAttack(sf::RenderWindow& window,
                            const sf::Font& font,
                            const soma::simulation::BattleSimulation& battle,
                            const CombatantState& actor,
                            const CombatantState& target,
                            const soma::content::AbilityDefinition& ability,
                            const soma::content::BattleLayoutDefinition& layout)
    {
        const auto friendly = actor.team == target.team;
        const auto& effect = friendly ? ability.friendlyEffect : ability.hostileEffect;
        const auto targetCount = static_cast<int>(simulation::abilityTargets(ability.targeting, actor, target, battle.combatants()).size());
        const auto targetAttributes = battle.effectiveAttributes(target);
        const auto actorAttributes = battle.effectiveAttributes(actor);
        const auto hitProbability = friendly || actor.id == target.id
                                        ? 100
                                        : target.misses > 0
                                              ? 0
                                              : std::clamp(100 -
                                                               (targetAttributes.dodge -
                                                                (actorAttributes.precision + ability.precisionModifier)) *
                                                                   2,
                                                           0,
                                                           100);

        std::vector<std::string> estimates;
        if (effect.damageFactor != 0.0)
        {
            const auto additionalFactor = effect.lessTargetsMoreDamage * (ability.targeting.targetCount - targetCount);
            auto conditionalFactor = effect.damageFactor + additionalFactor;
            if (target.markedTurns > 0)
            {
                conditionalFactor += effect.damageAgainstMarked;
                for (const auto& buff : actor.buffs)
                {
                    conditionalFactor += buff.damageAgainstMarked;
                }
            }
            if (target.fistOfRevengeTarget)
            {
                conditionalFactor += effect.damageAgainstRevengeMarked;
            }
            for (const auto& buff : actor.buffs)
            {
                if (buff.damageFromFatigue)
                {
                    conditionalFactor += static_cast<double>(actor.fatigue) / gameplay::maximumFatigue;
                }
            }
            const auto armour = effect.ignoreArmour ? 0 : targetAttributes.armour;
            const auto damageAfterArmour = [&battle, &actor, &target, armour, conditionalFactor](int damage) {
                const auto abilityDamage = static_cast<int>(damage * conditionalFactor);
                const auto modifiedDamage = battle.modifiedDamage(actor, target, abilityDamage);
                return modifiedDamage - static_cast<int>(std::round(armour / 100.0 * modifiedDamage));
            };
            const auto minimum = std::max(0, damageAfterArmour(actor.attributes.damageMinimum));
            const auto maximum = std::max(0, damageAfterArmour(actor.attributes.damageMaximum));
            const auto damageText = minimum == maximum ? std::to_string(minimum) : std::to_string(minimum) + "-" + std::to_string(maximum);
            estimates.push_back("Treffer: " + std::to_string(hitProbability) + "% | Schaden: " + damageText);
        }

        const auto statusProbability = [hitProbability](double baseProbability, int resistance) {
            const auto base = std::clamp(baseProbability, 0.0, 1.0);
            const auto resistanceFactor = 1.0 - std::clamp(resistance, 0, 100) / 100.0;
            return static_cast<int>(std::round(hitProbability * base * resistanceFactor));
        };
        std::vector<std::string> statusEstimates;
        if (effect.confusionRounds > 0)
        {
            statusEstimates.push_back("Verwirrung: " +
                                      std::to_string(statusProbability(effect.confusionProbability,
                                                                       target.secondaryAttributes.confusionResistance)) +
                                      "%");
        }
        if (effect.sleepProbability > 0.0)
        {
            auto probability = effect.sleepProbability;
            if (effect.increaseSleepProbabilityByFatigue)
            {
                probability += static_cast<double>(actor.fatigue) / gameplay::maximumFatigue;
            }
            statusEstimates.push_back("Schlaf: " +
                                      std::to_string(statusProbability(probability, target.secondaryAttributes.sleepResistance)) + "%");
        }
        if (effect.decayRounds > 0)
        {
            const auto resistance = effect.ignoreDecayResistance ? 0 : target.secondaryAttributes.decayResistance;
            statusEstimates.push_back("Verfall: " + std::to_string(statusProbability(1.0, resistance)) + "%");
        }
        if (effect.targetBuff.duration > 0 && !effect.targetBuff.positive)
        {
            statusEstimates.push_back("Schwächung: " +
                                      std::to_string(statusProbability(1.0, target.secondaryAttributes.debuffResistance)) + "%");
        }
        if (!statusEstimates.empty())
        {
            auto statusLine = statusEstimates.front();
            for (std::size_t index = 1; index < statusEstimates.size(); ++index)
            {
                statusLine += " | " + statusEstimates[index];
            }
            estimates.push_back(std::move(statusLine));
        }

        auto y = static_cast<float>(estimates.size() == 1 ? layout.expectedAttackSingleY : layout.expectedAttackMultipleY);
        for (const auto& value : estimates)
        {
            auto estimate = text(font, value, static_cast<unsigned int>(layout.expectedAttackTextSize), {}, sf::Color{200, 200, 130});
            estimate.setStyle(sf::Text::Bold);
            estimate.setPosition({static_cast<float>(layout.expectedAttackCenterX) - estimate.getLocalBounds().size.x / 2.0F, y});
            window.draw(estimate);
            y += static_cast<float>(layout.expectedAttackLineSpacing);
        }
    }
} // namespace soma::presentation
