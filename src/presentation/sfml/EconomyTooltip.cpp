#include "presentation/sfml/EconomyTooltip.hpp"

#include "gameplay/items/ItemRarity.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace soma::presentation
{
    namespace
    {
        struct TooltipLine
        {
            std::string value;
            sf::Color color{sf::Color::White};
            std::string suffix;
            sf::Color suffixColor{sf::Color::White};
        };

        float textAdvance(const sf::Text& text)
        {
            auto result = 0.0F;
            for (const auto& glyph : text.getShapedGlyphs())
            {
                result = std::max(result, glyph.position.x + glyph.glyph.advance);
            }
            return result;
        }

        void append(std::vector<TooltipLine>& lines, const std::string& name, int value, std::string_view suffix = {})
        {
            if (value != 0)
            {
                lines.push_back({name + (value > 0 ? ": +" : ": ") + std::to_string(value) + std::string{suffix}});
            }
        }

        void appendBonuses(std::vector<TooltipLine>& lines, const gameplay::AttributeBonuses& bonuses)
        {
            append(lines, "Rüstung", bonuses.armour);
            append(lines, "Maximales Leben", bonuses.maximumHealth);
            if (bonuses.damageMinimum != 0 || bonuses.damageMaximum != 0)
            {
                const auto value = bonuses.damageMinimum == bonuses.damageMaximum
                                       ? std::to_string(bonuses.damageMinimum)
                                       : std::to_string(bonuses.damageMinimum) + " bis " + std::to_string(bonuses.damageMaximum);
                lines.push_back({"Schaden: " + value});
            }
            append(lines, "Initiative", bonuses.initiative);
            append(lines, "Kritische Trefferchance", bonuses.criticalHitChance, "%");
            append(lines, "Ausweichen", bonuses.dodge);
            append(lines, "Präzision", bonuses.precision);
            append(lines, "Heilungsmodifikator", bonuses.healing, "%");
            append(lines, "Schlaf-Resistenz", bonuses.sleepResistance, "%");
            append(lines, "Verwirrungs-Resistenz", bonuses.confusionResistance, "%");
            append(lines, "Debuff-Resistenz", bonuses.debuffResistance, "%");
            append(lines, "Verfall-Resistenz", bonuses.decayResistance, "%");
        }

        void drawTooltipLines(sf::RenderWindow& window, const sf::Font& font, const std::vector<TooltipLine>& lines, sf::Vector2f mousePosition)
        {
            constexpr float horizontalPadding = 20.0F;
            auto contentWidth = 0.0F;
            for (std::size_t index = 0; index < lines.size(); ++index)
            {
                sf::Text measurement{font, sf::String::fromUtf8(lines.at(index).value.begin(), lines.at(index).value.end()), 18U};
                if (index == 0)
                {
                    measurement.setStyle(sf::Text::Style::Bold);
                }
                auto lineWidth = textAdvance(measurement);
                if (!lines.at(index).suffix.empty())
                {
                    sf::Text suffixMeasurement{
                        font, sf::String::fromUtf8(lines.at(index).suffix.begin(), lines.at(index).suffix.end()), 18U};
                    if (index == 0)
                    {
                        suffixMeasurement.setStyle(sf::Text::Style::Bold);
                    }
                    lineWidth += textAdvance(suffixMeasurement);
                }
                contentWidth = std::max(contentWidth, lineWidth);
            }
            const auto width = std::max(80.0F, contentWidth + horizontalPadding);
            const auto height = 18.0F + static_cast<float>(lines.size()) * 27.0F;
            auto position = mousePosition + sf::Vector2f{10.0F, -height - 10.0F};
            const auto view = window.getView();
            const auto viewOrigin = view.getCenter() - view.getSize() / 2.0F;
            const auto maximumX = std::max(viewOrigin.x + 5.0F, viewOrigin.x + view.getSize().x - width - 5.0F);
            const auto maximumY = std::max(viewOrigin.y + 5.0F, viewOrigin.y + view.getSize().y - height - 5.0F);
            position.x = std::clamp(position.x, viewOrigin.x + 5.0F, maximumX);
            position.y = std::clamp(position.y, viewOrigin.y + 5.0F, maximumY);
            sf::RectangleShape background{{width, height}};
            background.setPosition(position);
            background.setFillColor(sf::Color{0, 0, 0, 225});
            background.setOutlineColor(sf::Color{40, 40, 40});
            background.setOutlineThickness(2.0F);
            window.draw(background);
            for (std::size_t index = 0; index < lines.size(); ++index)
            {
                sf::Text text{font, sf::String::fromUtf8(lines.at(index).value.begin(), lines.at(index).value.end()), 18U};
                text.setPosition(position + sf::Vector2f{10.0F, 7.0F + static_cast<float>(index) * 27.0F});
                text.setFillColor(lines.at(index).color);
                if (index == 0)
                {
                    text.setStyle(sf::Text::Style::Bold);
                }
                window.draw(text);
                if (!lines.at(index).suffix.empty())
                {
                    sf::Text suffix{
                        font, sf::String::fromUtf8(lines.at(index).suffix.begin(), lines.at(index).suffix.end()), 18U};
                    suffix.setPosition({text.getPosition().x + textAdvance(text), text.getPosition().y});
                    suffix.setFillColor(lines.at(index).suffixColor);
                    if (index == 0)
                    {
                        suffix.setStyle(sf::Text::Style::Bold);
                    }
                    window.draw(suffix);
                }
            }
        }

        std::vector<TooltipLine> tooltipLines(const gameplay::ItemDefinition& definition)
        {
            TooltipLine title{definition.name, sf::Color{255, 165, 0}};
            if (definition.category != gameplay::ItemCategory::currency)
            {
                title.suffix = " (" + std::string{gameplay::itemRarityName(definition.level)} + ")";
                title.suffixColor = sf::Color{160, 160, 160};
            }
            std::vector<TooltipLine> lines{std::move(title)};
            if (!definition.description.empty())
            {
                lines.push_back({definition.description});
                return lines;
            }
            if (const auto* equipment = std::get_if<gameplay::EquipmentItem>(&definition.properties))
            {
                if (equipment->addsMissOnHighDamage)
                {
                    lines.push_back({"Bei mindestens 10 Schaden: Erhalte einen Fehlschuss"});
                }
                if (equipment->healsOnPass)
                {
                    lines.push_back({"Beim Passen: Heile 3 Leben und verliere 3 Müdigkeit"});
                }
                appendBonuses(lines, equipment->bonuses);
            }
            else if (const auto* consumable = std::get_if<gameplay::ConsumableItem>(&definition.properties))
            {
                if (consumable->healing != 0)
                {
                    lines.push_back({"Heilt " + std::to_string(consumable->healing) + " Leben", sf::Color{0, 170, 0}});
                }
                if (consumable->duration > 0)
                {
                    lines.push_back({"Für " + std::to_string(consumable->duration) + (consumable->duration == 1 ? " Runde:" : " Runden:")});
                    appendBonuses(lines, consumable->temporaryBonuses);
                }
            }
            return lines;
        }
    } // namespace

    void drawItemTooltip(sf::RenderWindow& window, const sf::Font& font, const gameplay::ItemDefinition& definition, sf::Vector2f mousePosition)
    {
        drawTooltipLines(window, font, tooltipLines(definition), mousePosition);
    }

    void drawBonusTooltip(
        sf::RenderWindow& window, const sf::Font& font, std::string_view title, const gameplay::AttributeBonuses& bonuses, sf::Vector2f mousePosition)
    {
        std::vector<TooltipLine> lines{{std::string{title}, sf::Color{255, 165, 0}}};
        appendBonuses(lines, bonuses);
        drawTooltipLines(window, font, lines, mousePosition);
    }

    void drawRuleFragmentTooltip(sf::RenderWindow& window,
                                 const sf::Font& font,
                                 const content::RuleFragmentDefinition& definition,
                                 sf::Vector2f mousePosition)
    {
        static constexpr std::array names{"Auslöser", "Subjekt", "Effekt", "Modifikator"};
        const auto type = static_cast<std::size_t>(definition.type);
        const auto supportsTriggered = definition.allowedModes.at(0);
        const auto supportsStanding = definition.allowedModes.at(1);
        const auto compatibility = supportsTriggered && supportsStanding ? "Für ausgelöste und stehende Regeln"
                                   : supportsTriggered                   ? "Nur für ausgelöste Regeln"
                                                                         : "Nur für stehende Regeln";
        drawTooltipLines(
            window,
            font,
            {{"Regelfragment: " + std::string{names.at(type)}, sf::Color{255, 165, 0}}, {definition.text}, {compatibility, sf::Color{180, 180, 180}}},
            mousePosition);
    }
} // namespace soma::presentation
