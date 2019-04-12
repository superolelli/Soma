#pragma once

#include <SFML/Graphics.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../../Framework/Patterns/singleton.hpp"
#include "BackgroundIDs.hpp"


#define g_pTextures CTextureManager::Get()
class CTextureManager : public TSingleton<CTextureManager>
{
public:
	//Loads all textures
	void LoadTextures();

	sf::Texture bangBackgrounds[12];

	sf::Texture abilities[4][4];

	sf::Texture currentAbilityFrame;

	sf::Texture healthBarFrame;
	sf::Texture healthBar;
	sf::Texture buff;
	sf::Texture debuff;
	sf::Texture confused;
	sf::Texture marked;
	sf::Texture sleeping;

	sf::Texture abilityAnnouncementBanner;
	sf::Texture abilityTargetMarker;
	sf::Texture turnMarker;
};