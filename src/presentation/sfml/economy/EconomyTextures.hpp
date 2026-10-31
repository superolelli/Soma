#pragma once

#include "content/battle/BattleContent.hpp"
#include "content/economy/EconomyContent.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <functional>
#include <unordered_map>

namespace soma::presentation
{
    /// Preloads the shared texture atlas used by inventory, skill, shop, and rule-related screens.
    class EconomyTextures
    {
      public:
        explicit EconomyTextures(const content::EconomyContent& content, std::function<void()> loadedTexture = {});
        EconomyTextures(const content::EconomyContent& content,
                        const content::BattleContent& battleContent,
                        std::function<void()> loadedTexture = {});

        [[nodiscard]] const sf::Texture& at(const gameplay::AssetId& id) const;
        [[nodiscard]] const sf::Texture& at(std::string_view id) const;

      private:
        void load(const gameplay::AssetId& id, const std::filesystem::path& path, const std::function<void()>& loadedTexture);

        std::unordered_map<gameplay::AssetId, sf::Texture> textures_;
    };
} // namespace soma::presentation
