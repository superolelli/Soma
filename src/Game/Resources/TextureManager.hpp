#pragma once

#include <SFML/Graphics.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../../Framework/Patterns/singleton.hpp"

#define g_pTextures CTextureManager::Get()
class CTextureManager : public TSingleton<CTextureManager>
{
public:
	//Loads all textures
	void LoadTextures();

	
};