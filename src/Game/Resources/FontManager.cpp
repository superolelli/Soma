#include "FontManager.hpp"



void CFontManager::LoadFonts()
{
	f_kingArthur.loadFromFile("Data/Fonts/kingArthur.ttf");
	f_calibri.loadFromFile("Data/Fonts/calibri.ttf");
	f_arial.loadFromFile("Data/Fonts/arial.ttf");
	f_plantc.loadFromFile("Data/Fonts/plantc.ttf");
	f_openSans.loadFromFile("Data/Fonts/openSans.ttf");
	f_trajan.loadFromFile("Data/Fonts/trajan.ttf");
	f_boris.loadFromFile("Data/Fonts/boris.ttf");
	f_tiza.loadFromFile("Data/Fonts/tiza.ttf");
	f_shanghai.loadFromFile("Data/Fonts/shanghai.ttf");
	f_blackwoodCastle.loadFromFile("Data/Fonts/blackwoodCastle.ttf");
}

