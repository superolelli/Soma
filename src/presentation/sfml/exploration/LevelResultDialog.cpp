#include "presentation/sfml/exploration/LevelResultDialog.hpp"

namespace soma::presentation
{
    namespace
    {
        sf::Vector2f vector(const content::Point& point)
        {
            return {static_cast<float>(point.x), static_cast<float>(point.y)};
        }

        sf::Text label(const sf::Font& font, const std::string& value, unsigned int size, sf::Vector2f position)
        {
            sf::Text result{font, sf::String::fromUtf8(value.begin(), value.end()), size};
            result.setPosition(position);
            result.setFillColor(sf::Color::White);
            result.setOutlineColor(sf::Color::Black);
            result.setOutlineThickness(4.0F);
            return result;
        }

        std::string descriptions(const simulation::LevelRewards& rewards)
        {
            std::string result;
            for (const auto& achievement : rewards.achievements)
            {
                result += achievement.description + '\n';
            }
            return result;
        }

        std::string amounts(const simulation::LevelRewards& rewards, bool cards)
        {
            std::string result;
            for (const auto& achievement : rewards.achievements)
            {
                result += std::to_string(cards ? achievement.cards : achievement.dice) + '\n';
            }
            return result;
        }
    } // namespace

    LevelResultDialog::LevelResultDialog(const content::WorldContent& world,
                                         const content::EconomyContent& economy,
                                         const content::RuleContent& rules,
                                         const EconomyTextures& textures,
                                         std::string_view campaign,
                                         bool completed,
                                         simulation::LevelRewards rewards,
                                         std::vector<gameplay::ItemId> unlockedItems,
                                         std::vector<content::RuleFragmentType> unlockedRuleTypes)
        : layout_(world.presentation.levelResultDialog)
        , panelTexture_(world.assets.at(gameplay::AssetId{campaign == "kutschfahrt" ? "level_result.kutschfahrt.panel" : "level_result.bang.panel"}))
        , buttonTexture_(world.assets.at(gameplay::AssetId{"level_result.button"}))
        , titleFont_(economy.assets.at(gameplay::AssetId{"font.price"}))
        , textFont_(economy.assets.at(gameplay::AssetId{"font.title"}))
        , economy_(economy)
        , textures_(textures)
        , completed_(completed)
        , rewards_(std::move(rewards))
        , unlockedItems_(std::move(unlockedItems))
        , unlockedRuleTypes_(std::move(unlockedRuleTypes))
    {
        for (const auto type : unlockedRuleTypes_)
        {
            ruleTextures_.emplace(type, sf::Texture{rules.fragmentSprites.at(type)});
        }
    }

    bool LevelResultDialog::clicked(sf::Vector2f position) const
    {
        return buttonBounds().contains(position);
    }

    void LevelResultDialog::render(sf::RenderWindow& window, sf::Vector2f mousePosition) const
    {
        window.draw(sf::Sprite{panelTexture_});
        window.draw(label(titleFont_, completed_ ? "Level beendet!" : "Versagt!", 50U, vector(layout_.title)));
        window.draw(label(textFont_, descriptions(rewards_), 32U, vector(layout_.rewardDescription)));
        window.draw(label(textFont_, amounts(rewards_, true), 32U, vector(layout_.rewardCards)));
        window.draw(label(textFont_, amounts(rewards_, false), 32U, vector(layout_.rewardDice)));
        window.draw(label(titleFont_, std::to_string(rewards_.totalCards()) + "x", 40U, vector(layout_.totalCards)));
        window.draw(label(titleFont_, std::to_string(rewards_.totalDice()) + "x", 40U, vector(layout_.totalDice)));
        if (!unlockedItems_.empty() || !unlockedRuleTypes_.empty())
        {
            window.draw(label(textFont_, "Freigeschaltet!", 41U, vector(layout_.unlockTitle)));
            for (std::size_t index = 0; index < unlockedItems_.size(); ++index)
            {
                const auto column = static_cast<float>(index % 2U);
                const auto row = static_cast<float>(index / 2U);
                sf::Sprite icon{textures_.at(economy_.items.at(unlockedItems_.at(index)).icon)};
                icon.setPosition(vector(layout_.firstUnlockedItem) +
                                 sf::Vector2f{column * layout_.unlockedItemSpacing.x, row * layout_.unlockedItemSpacing.y});
                window.draw(icon);
            }
            for (std::size_t ruleIndex = 0; ruleIndex < unlockedRuleTypes_.size(); ++ruleIndex)
            {
                const auto index = unlockedItems_.size() + ruleIndex;
                const auto column = static_cast<float>(index % 2U);
                const auto row = static_cast<float>(index / 2U);
                sf::Sprite icon{ruleTextures_.at(unlockedRuleTypes_.at(ruleIndex))};
                icon.setPosition(vector(layout_.firstUnlockedItem) +
                                 sf::Vector2f{column * layout_.unlockedItemSpacing.x, row * layout_.unlockedItemSpacing.y});
                window.draw(icon);
            }
        }

        const auto frameWidth = static_cast<int>(buttonTexture_.getSize().x / 3U);
        const auto hovered = buttonBounds().contains(mousePosition);
        sf::Sprite button{buttonTexture_};
        button.setTextureRect({{hovered ? frameWidth : 0, 0}, {frameWidth, static_cast<int>(buttonTexture_.getSize().y)}});
        button.setPosition(vector(layout_.continueButton));
        window.draw(button);
        auto buttonText = label(textFont_, "Weiter", 40U, {});
        const auto bounds = buttonText.getLocalBounds();
        buttonText.setOrigin({bounds.position.x + bounds.size.x / 2.0F, bounds.position.y + bounds.size.y / 2.0F});
        buttonText.setPosition(buttonBounds().getCenter());
        window.draw(buttonText);
    }

    sf::FloatRect LevelResultDialog::buttonBounds() const
    {
        return {vector(layout_.continueButton),
                {static_cast<float>(buttonTexture_.getSize().x / 3U), static_cast<float>(buttonTexture_.getSize().y)}};
    }
} // namespace soma::presentation
