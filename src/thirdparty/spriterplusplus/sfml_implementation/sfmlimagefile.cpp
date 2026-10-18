#include "sfmlimagefile.h"

#include "../global/settings.h"
#include "../objectinfo/universalobjectinterface.h"

#include <algorithm>

namespace SpriterEngine
{

    SfmlImageFile::SfmlImageFile(std::string initialFilePath,
                                 point initialDefaultPivot,
                                 sf::RenderTarget* validRenderWindow,
                                 const SfmlRenderSettings* initialRenderSettings)
        : ImageFile(initialFilePath, initialDefaultPivot)
        , sprite(texture)
        , renderWindow(validRenderWindow)
        , renderSettings(initialRenderSettings)
    {
        initializeFile();
    }

    void SfmlImageFile::initializeFile()
    {
        if (texture.loadFromFile(path()))
        {
            texture.setSmooth(true);
            sprite.setTexture(texture, true);
        }
        else
        {
            Settings::Settings::error(
                "SfmlImageFile::initializeFile - sfml texture unable to load file from path \"" +
                path() + "\"");
        }
    }

    void SfmlImageFile::renderSprite(UniversalObjectInterface* spriteInfo)
    {
        const auto alpha =
            static_cast<std::uint8_t>(std::clamp(spriteInfo->getAlpha(), 0.0, 1.0) * 255.0);
        const auto color = renderSettings != nullptr ? renderSettings->color : sf::Color::White;
        sprite.setColor(sf::Color(color.r, color.g, color.b, static_cast<std::uint8_t>(color.a * alpha / 255U)));
        const auto offset = renderSettings != nullptr ? renderSettings->offset : sf::Vector2f{};
        sprite.setPosition({static_cast<float>(spriteInfo->getPosition().x) + offset.x,
                            static_cast<float>(spriteInfo->getPosition().y) + offset.y});
        sprite.setRotation(sf::degrees(toDegrees(spriteInfo->getAngle())));
        sprite.setScale({static_cast<float>(spriteInfo->getScale().x),
                         static_cast<float>(spriteInfo->getScale().y)});
        sprite.setOrigin({static_cast<float>(spriteInfo->getPivot().x * texture.getSize().x),
                          static_cast<float>(spriteInfo->getPivot().y * texture.getSize().y)});
        if (renderSettings != nullptr && renderSettings->shader != nullptr)
        {
            sf::RenderStates states;
            states.shader = renderSettings->shader;
            renderWindow->draw(sprite, states);
        }
        else
        {
            renderWindow->draw(sprite);
        }
    }

} // namespace SpriterEngine
