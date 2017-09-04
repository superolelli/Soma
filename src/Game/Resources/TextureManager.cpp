#include "TextureManager.hpp"



void CTextureManager::LoadTextures()
{
	//backgrounds
	bangBackgrounds[room_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_raum1.png");
	bangBackgrounds[corridor_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_1.png");
	bangBackgrounds[door_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_1.png");
	bangBackgrounds[door_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_2.png");
	bangBackgrounds[end].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_ende.png");

}


