#include "presentation/sfml/EconomyTextures.hpp"

#include <array>
#include <stdexcept>

namespace soma::presentation
{
    EconomyTextures::EconomyTextures(const content::EconomyContent& content, std::function<void()> loadedTexture)
    {
        for (const auto& [id, path] : content.assets)
        {
            load(id, path, loadedTexture);
        }
    }

    EconomyTextures::EconomyTextures(const content::EconomyContent& content,
                                     const content::BattleContent& battleContent,
                                     std::function<void()> loadedTexture)
        : EconomyTextures(content, loadedTexture)
    {
        static constexpr std::array statusAssets{
            std::pair{"status.confused", "status_confused"},
            std::pair{"status.decay", "status_decay"},
            std::pair{"status.marked", "status_marked"},
            std::pair{"status.sleeping", "status_sleeping"},
            std::pair{"status.miss", "status_missed"},
            std::pair{"status.dynamite", "status_dynamite"},
            std::pair{"status.bounty", "status_bounty"},
            std::pair{"status.riposte", "status_riposte"},
            std::pair{"status.rage", "status_rage"},
            std::pair{"status.hacked", "status_hacked"},
            std::pair{"status.transfusion", "status_transfusion"},
            std::pair{"status.panic", "status_panic"}};
        for (const auto& [alias, battleAsset] : statusAssets)
        {
            load(gameplay::AssetId{alias}, battleContent.assets.at(battleAsset), loadedTexture);
        }
    }

    void EconomyTextures::load(const gameplay::AssetId& id,
                               const std::filesystem::path& path,
                               const std::function<void()>& loadedTexture)
    {
        if (path.extension() != ".png")
        {
            return;
        }
        sf::Texture texture;
        if (!texture.loadFromFile(path))
        {
            throw std::runtime_error("Could not load economy texture: " + path.string());
        }
        textures_.emplace(id, std::move(texture));
        if (loadedTexture)
        {
            loadedTexture();
        }
    }

    const sf::Texture& EconomyTextures::at(const gameplay::AssetId& id) const
    {
        return textures_.at(id);
    }

    const sf::Texture& EconomyTextures::at(std::string_view id) const
    {
        return at(gameplay::AssetId{std::string{id}});
    }
} // namespace soma::presentation
