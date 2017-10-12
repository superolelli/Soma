#include "TextureManager.hpp"



void CTextureManager::LoadTextures()
{
	//backgrounds
	bangBackgrounds[room_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_raum1.png");
	bangBackgrounds[corridor_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_1.png");
	bangBackgrounds[door_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_1.png");
	bangBackgrounds[door_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_2.png");
	bangBackgrounds[end].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_ende.png");

	abilities[0].loadFromFile("Data/Sprites/Abilities/marshmallow_flash.png");
	abilities[1].loadFromFile("Data/Sprites/Abilities/alle_auf_einen.png");
	abilities[2].loadFromFile("Data/Sprites/Abilities/reparieren.png");
	abilities[3].loadFromFile("Data/Sprites/Abilities/verarschen.png");

	currentAbilityFrame.loadFromFile("Data/Sprites/Abilities/currentAbilityFrame.png");

}


