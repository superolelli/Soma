#pragma once

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Exception.hpp>

// Temporary resources for legacy objects that were default-constructible in SFML 2.
// Prefer injecting the real resource in constructors when these classes are refactored.
namespace SfmlCompat
{
inline const sf::Font& defaultFont()
{
    static const sf::Font font = []
    {
        sf::Font loadedFont;
        if (!loadedFont.openFromFile("Data/Fonts/arial.ttf"))
            throw sf::Exception("Could not load the fallback UI font Data/Fonts/arial.ttf");
        return loadedFont;
    }();
    return font;
}

inline const sf::Texture& defaultTexture()
{
    static const sf::Texture texture;
    return texture;
}

inline const sf::SoundBuffer& defaultSoundBuffer()
{
    static const sf::SoundBuffer buffer;
    return buffer;
}
}
