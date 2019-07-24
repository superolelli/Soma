#pragma once

#include <SFML/Graphics.hpp>
#include "../../Framework/Patterns/singleton.hpp"

#define g_pFonts CFontManager::Get()
class CFontManager : public TSingleton<CFontManager>
{
public:
	//Loads all fonts
	void LoadFonts();

	//returns a font
	sf::Font f_kingArthur;
	sf::Font f_calibri;
	sf::Font f_arial;
	sf::Font f_plantc;
	sf::Font f_openSans;
	sf::Font f_trajan;
	sf::Font f_boris;

};