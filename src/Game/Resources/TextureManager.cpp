#include "TextureManager.hpp"



void CTextureManager::LoadTextures()
{
	//backgrounds
	bangBackgrounds[room_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_raum1.png");
	bangBackgrounds[corridor_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_1.png");
	bangBackgrounds[door_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_1.png");
	bangBackgrounds[door_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_2.png");
	bangBackgrounds[end].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_ende.png");

	abilities[0][0].loadFromFile("Data/Sprites/Abilities/marshmallow_flash.png");
	abilities[0][3].loadFromFile("Data/Sprites/Abilities/alle_auf_einen.png");
	abilities[0][2].loadFromFile("Data/Sprites/Abilities/reparieren.png");
	abilities[0][1].loadFromFile("Data/Sprites/Abilities/verarschen.png");

	abilities[1][0].loadFromFile("Data/Sprites/Abilities/gummi_arm.png");
	abilities[1][1].loadFromFile("Data/Sprites/Abilities/whole_body_kick.png");
	abilities[1][2].loadFromFile("Data/Sprites/Abilities/hack_into_the_system.png");
	abilities[1][3].loadFromFile("Data/Sprites/Abilities/poetry_slam.png");

	abilities[2][0].loadFromFile("Data/Sprites/Abilities/indomitable_happiness.png");
	abilities[2][1].loadFromFile("Data/Sprites/Abilities/word_gush.png");
	abilities[2][2].loadFromFile("Data/Sprites/Abilities/side_of_nose_blow.png");
	abilities[2][3].loadFromFile("Data/Sprites/Abilities/deathhug.png");

	abilities[3][0].loadFromFile("Data/Sprites/Abilities/fist_of_revenge.png");
	abilities[3][1].loadFromFile("Data/Sprites/Abilities/nose_ram.png");
	abilities[3][2].loadFromFile("Data/Sprites/Abilities/harte_zerstörung.png");
	abilities[3][3].loadFromFile("Data/Sprites/Abilities/horrible_grin.png");

	currentAbilityFrame.loadFromFile("Data/Sprites/Abilities/currentAbilityFrame.png");

	healthBarFrame.loadFromFile("Data/Sprites/Battle/health_bar_frame.png");
	healthBar.loadFromFile("Data/Sprites/Battle/health_bar.png");
	buff.loadFromFile("Data/Sprites/Battle/buff.png");
	debuff.loadFromFile("Data/Sprites/Battle/debuff.png");
	confused.loadFromFile("Data/Sprites/Battle/confused.png");
	marked.loadFromFile("Data/Sprites/Battle/marked.png");
	sleeping.loadFromFile("Data/Sprites/Battle/sleeping.png");

	abilityAnnouncementBanner.loadFromFile("Data/Sprites/Battle/ability_announcement.png");

	abilityTargetMarker.loadFromFile("Data/Sprites/Battle/battle_marker.png");
	turnMarker.loadFromFile("Data/Sprites/Battle/turn_marker.png");

}


