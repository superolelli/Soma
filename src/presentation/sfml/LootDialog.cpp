#include "presentation/sfml/LootDialog.hpp"

#include "presentation/sfml/AudioSettings.hpp"
#include "presentation/sfml/EconomyTooltip.hpp"

#include <algorithm>

namespace soma::presentation
{
    namespace
    {
        sf::Vector2f vector(const content::Point& point)
        {
            return {static_cast<float>(point.x), static_cast<float>(point.y)};
        }

        sf::Text label(const sf::Font& font, const std::string& value, unsigned int size)
        {
            sf::Text result{font, sf::String::fromUtf8(value.begin(), value.end()), size};
            result.setFillColor(sf::Color::White);
            result.setOutlineColor(sf::Color::Black);
            result.setOutlineThickness(2.0F);
            return result;
        }

        void centerAt(sf::Text& text, sf::Vector2f center)
        {
            const auto bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x / 2.0F, bounds.position.y + bounds.size.y / 2.0F});
            text.setPosition(center);
        }
    } // namespace

    LootDialog::LootDialog(const content::WorldContent& world,
                           const content::EconomyContent& economy,
                           const content::RuleContent& rules,
                           const EconomyTextures& textures,
                           EconomySoundPlayer& sounds,
                           game::EconomyService& economyService,
                           game::RuleBuilderService& ruleService,
                           std::string title,
                           std::optional<std::filesystem::path> openingSound,
                           simulation::LootBundle loot)
        : world_(world)
        , economy_(economy)
        , rules_(rules)
        , textures_(textures)
        , sounds_(sounds)
        , economyService_(economyService)
        , ruleService_(ruleService)
        , layout_(world.presentation.lootDialog)
        , panelTexture_(world.assets.at(gameplay::AssetId{"loot_dialog.panel"}))
        , closeTexture_(world.assets.at(gameplay::AssetId{"loot_dialog.close"}))
        , takeAllTexture_(world.assets.at(gameplay::AssetId{"loot_dialog.take_all"}))
        , font_(economy.assets.at(gameplay::AssetId{"font.interface"}))
        , itemSoundBuffer_(world.assets.at(gameplay::AssetId{"loot_dialog.item_sound"}))
        , title_(std::move(title))
        , ruleFragment_(std::move(loot.ruleFragment))
    {
        items_.reserve(loot.items.size());
        for (auto& item : loot.items)
        {
            items_.emplace_back(std::move(item));
        }
        for (const auto& [type, path] : rules_.fragmentSprites)
        {
            ruleTextures_.emplace(type, sf::Texture{path});
        }
        if (openingSound)
        {
            openingSoundBuffer_.emplace(*openingSound);
            openingSound_ = std::make_unique<sf::Sound>(*openingSoundBuffer_);
            openingSound_->setVolume(AudioSettings::sound());
            openingSound_->play();
        }
    }

    void LootDialog::handleClick(sf::Vector2f position)
    {
        const auto closeSize = sf::Vector2f{static_cast<float>(closeTexture_.getSize().x / 3U), static_cast<float>(closeTexture_.getSize().y)};
        if (sf::FloatRect{vector(layout_.closeButton), closeSize}.contains(position))
        {
            sounds_.click();
            closed_ = true;
            return;
        }
        const auto takeAllSize = sf::Vector2f{static_cast<float>(takeAllTexture_.getSize().x / 3U), static_cast<float>(takeAllTexture_.getSize().y)};
        if (sf::FloatRect{vector(layout_.takeAllButton), takeAllSize}.contains(position))
        {
            sounds_.click();
            sounds_.lootTakeAll();
            takeAll();
            return;
        }
        for (std::size_t index = 0; index < items_.size(); ++index)
        {
            if (items_.at(index) && itemBounds(index).contains(position))
            {
                static_cast<void>(take(index));
                return;
            }
        }
        if (ruleFragment_ && itemBounds(items_.size()).contains(position))
        {
            const auto result = ruleService_.grant(*ruleFragment_);
            if (!result.hasValue())
            {
                error_ = result.error();
            }
            else
            {
                ruleFragment_.reset();
            }
        }
    }

    bool LootDialog::clickableAt(sf::Vector2f position) const
    {
        const auto closeSize = sf::Vector2f{static_cast<float>(closeTexture_.getSize().x / 3U), static_cast<float>(closeTexture_.getSize().y)};
        const auto takeAllSize = sf::Vector2f{static_cast<float>(takeAllTexture_.getSize().x / 3U), static_cast<float>(takeAllTexture_.getSize().y)};
        if (sf::FloatRect{vector(layout_.closeButton), closeSize}.contains(position) ||
            sf::FloatRect{vector(layout_.takeAllButton), takeAllSize}.contains(position))
        {
            return true;
        }
        for (std::size_t index = 0; index < items_.size(); ++index)
        {
            if (items_.at(index) && itemBounds(index).contains(position))
            {
                return true;
            }
        }
        if (ruleFragment_ && itemBounds(items_.size()).contains(position))
        {
            return true;
        }
        return false;
    }

    void LootDialog::render(sf::RenderWindow& window, sf::Vector2f mousePosition) const
    {
        sf::RectangleShape dimmer{vector(world_.presentation.referenceSize)};
        dimmer.setFillColor({0, 0, 0, 120});
        window.draw(dimmer);

        sf::Sprite panel{panelTexture_};
        panel.setPosition(vector(layout_.position));
        window.draw(panel);

        auto title = label(font_, title_, 20U);
        title.setFillColor(sf::Color::Black);
        title.setOutlineThickness(0.0F);
        title.setStyle(sf::Text::Style::Bold);
        centerAt(title, vector(layout_.titleCenter));
        window.draw(title);

        for (std::size_t index = 0; index < items_.size(); ++index)
        {
            if (!items_.at(index))
            {
                continue;
            }
            const auto& grant = *items_.at(index);
            const auto& definition = economy_.items.at(grant.item.definition);
            const auto bounds = itemBounds(index);
            sf::RectangleShape color{bounds.size - sf::Vector2f{3.0F, 3.0F}};
            color.setPosition(bounds.position + sf::Vector2f{2.0F, 2.0F});
            const auto itemColor = gameplay::normalizedItemColor(grant.item.color);
            color.setFillColor({itemColor.red, itemColor.green, itemColor.blue});
            window.draw(color);
            sf::Sprite icon{textures_.at(definition.icon)};
            icon.setPosition(bounds.position);
            window.draw(icon);
            if (grant.quantity > 1)
            {
                auto quantity = label(font_, std::to_string(grant.quantity), 20U);
                quantity.setPosition(itemBounds(index).position + vector(layout_.quantityOffset));
                window.draw(quantity);
            }
        }
        if (ruleFragment_)
        {
            const auto& definition = rules_.fragments.at(*ruleFragment_);
            sf::Sprite icon{ruleTextures_.at(definition.type)};
            const auto bounds = itemBounds(items_.size());
            const auto textureSize = icon.getTexture().getSize();
            const auto scale = std::min(bounds.size.x / static_cast<float>(textureSize.x), bounds.size.y / static_cast<float>(textureSize.y));
            icon.setScale({scale, scale});
            icon.setPosition(bounds.position);
            window.draw(icon);
        }

        sf::Sprite close{closeTexture_};
        const auto closeWidth = static_cast<int>(closeTexture_.getSize().x / 3U);
        const auto closeHovered =
            sf::FloatRect{vector(layout_.closeButton), {static_cast<float>(closeWidth), static_cast<float>(closeTexture_.getSize().y)}}.contains(
                mousePosition);
        close.setTextureRect({{closeHovered ? closeWidth : 0, 0}, {closeWidth, static_cast<int>(closeTexture_.getSize().y)}});
        close.setPosition(vector(layout_.closeButton));
        window.draw(close);
        sf::Sprite takeAll{takeAllTexture_};
        const auto takeAllWidth = static_cast<int>(takeAllTexture_.getSize().x / 3U);
        const auto takeAllHovered =
            sf::FloatRect{vector(layout_.takeAllButton), {static_cast<float>(takeAllWidth), static_cast<float>(takeAllTexture_.getSize().y)}}
                .contains(mousePosition);
        takeAll.setTextureRect({{takeAllHovered ? takeAllWidth : 0, 0}, {takeAllWidth, static_cast<int>(takeAllTexture_.getSize().y)}});
        takeAll.setPosition(vector(layout_.takeAllButton));
        window.draw(takeAll);

        auto takeAllText = label(font_, "Alles\nnehmen", 21U);
        centerAt(takeAllText,
                 vector(layout_.takeAllButton) +
                     sf::Vector2f{static_cast<float>(takeAllTexture_.getSize().x / 6U), static_cast<float>(takeAllTexture_.getSize().y) / 2.0F});
        window.draw(takeAllText);
        if (!error_.empty())
        {
            auto error = label(font_, error_, 17U);
            error.setFillColor(sf::Color{220, 50, 50});
            centerAt(error, vector(layout_.errorCenter));
            window.draw(error);
        }
        for (std::size_t index = 0; index < items_.size(); ++index)
        {
            if (items_.at(index) && itemBounds(index).contains(mousePosition))
            {
                drawItemTooltip(window, font_, economy_.items.at(items_.at(index)->item.definition), mousePosition);
                break;
            }
        }
        if (ruleFragment_ && itemBounds(items_.size()).contains(mousePosition))
        {
            drawRuleFragmentTooltip(window, font_, rules_.fragments.at(*ruleFragment_), mousePosition);
        }
    }

    bool LootDialog::closed() const noexcept
    {
        return closed_;
    }

    sf::FloatRect LootDialog::itemBounds(std::size_t index) const
    {
        const auto column = static_cast<int>(index) % layout_.columns;
        const auto row = static_cast<int>(index) / layout_.columns;
        return {vector(layout_.firstItem) +
                    sf::Vector2f{static_cast<float>(column * layout_.itemSpacing.x), static_cast<float>(row * layout_.itemSpacing.y)},
                vector(layout_.itemSize)};
    }

    bool LootDialog::take(std::size_t index, bool playFeedback)
    {
        const auto& grant = *items_.at(index);
        const auto& definition = economy_.items.at(grant.item.definition);
        const auto result = economyService_.grant(definition, grant.quantity, grant.item.color);
        if (!result.hasValue())
        {
            error_ = result.error();
            return false;
        }
        items_.at(index).reset();
        error_.clear();
        if (playFeedback)
        {
            feedbackSound_ = std::make_unique<sf::Sound>(itemSoundBuffer_);
            feedbackSound_->setVolume(AudioSettings::sound());
            feedbackSound_->play();
        }
        return true;
    }

    void LootDialog::takeAll()
    {
        for (std::size_t index = 0; index < items_.size(); ++index)
        {
            if (items_.at(index) && !take(index, false))
            {
                return;
            }
        }
        if (ruleFragment_)
        {
            const auto result = ruleService_.grant(*ruleFragment_);
            if (!result.hasValue())
            {
                error_ = result.error();
                return;
            }
            ruleFragment_.reset();
        }
        closed_ = true;
    }
} // namespace soma::presentation
