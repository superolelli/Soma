#include "FontManager.hpp"



void CFontManager::LoadFonts()
{
	f_kingArthur.openFromFile("Data/Fonts/kingArthur.ttf");
	f_calibri.openFromFile("Data/Fonts/calibri.ttf");
	f_arial.openFromFile("Data/Fonts/arial.ttf");
	f_plantc.openFromFile("Data/Fonts/plantc.ttf");
	f_openSans.openFromFile("Data/Fonts/openSans.ttf");
	f_trajan.openFromFile("Data/Fonts/trajan.ttf");
	f_boris.openFromFile("Data/Fonts/boris.ttf");
	f_tiza.openFromFile("Data/Fonts/tiza.ttf");
	f_shanghai.openFromFile("Data/Fonts/shanghai.ttf");
	f_blackwoodCastle.openFromFile("Data/Fonts/blackwoodCastle.ttf");
	f_showcard.openFromFile("Data/Fonts/showcard.ttf");
}

