#include "TextureManager.hpp"



void CTextureManager::LoadTextures()
{
	statusBar.loadFromFile("Data/Sprites/Panels & Buttons/status_bar.png");

	//backgrounds
	bangBackgrounds[room_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_raum1.png");
	bangBackgrounds[corridor_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_1.png");
	bangBackgrounds[corridor_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_2.png");
	bangBackgrounds[corridor_3].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_3.png");
	bangBackgrounds[corridor_4].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_4.png");
	bangBackgrounds[corridor_5].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_5.png");
	bangBackgrounds[corridor_6].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_6.png");
	bangBackgrounds[corridor_7].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_7.png");
	bangBackgrounds[corridor_8].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_8.png");
	bangBackgrounds[door_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_1.png");
	bangBackgrounds[door_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür_2.png");
	bangBackgrounds[end].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_ende.png");

	bangDoor.loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_tür.png");

	bangLevelFinishedPanel.loadFromFile("Data/Sprites/Panels & Buttons/Bang/level_finished.png");
	bangGenericButton.loadFromFile("Data/Sprites/Panels & Buttons/Bang/generic_button.png");

	mainRoomBackgrounds[0].loadFromFile("Data/Sprites/Backgrounds/Main Room/main_room_1.png");
	mainRoomBackgrounds[1].loadFromFile("Data/Sprites/Backgrounds/Main Room/main_room_2.png");
	mainRoomBackgrounds[2].loadFromFile("Data/Sprites/Backgrounds/Main Room/main_room_3.png");
	mainRoomBackgrounds[3].loadFromFile("Data/Sprites/Backgrounds/Main Room/main_room_4.png");

	mainRoomDoors[MainRoomDoors::door_bang].loadFromFile("Data/Sprites/Backgrounds/Main Room/door_wood.png");
	mainRoomDoors[MainRoomDoors::door_kutschfahrt].loadFromFile("Data/Sprites/Backgrounds/Main Room/door_stone.png");
	mainRoomDoors[MainRoomDoors::door_tichu].loadFromFile("Data/Sprites/Backgrounds/Main Room/door_tichu.png");

	mainRoomSigns[0].loadFromFile("Data/Sprites/Backgrounds/Main Room/level_sign_1.png");
	mainRoomSigns[1].loadFromFile("Data/Sprites/Backgrounds/Main Room/level_sign_2.png");
	mainRoomSigns[2].loadFromFile("Data/Sprites/Backgrounds/Main Room/level_sign_3.png");

	mainRoomRoots.loadFromFile("Data/Sprites/Backgrounds/Main Room/roots.png");

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
	abilityPanel.loadFromFile("Data/Sprites/Abilities/ability_panel.png");

	combatantInformationPanel.loadFromFile("Data/Sprites/Battle/combatant_information_panel.png");

	healthBarFrame.loadFromFile("Data/Sprites/Battle/health_bar_frame.png");
	healthBar.loadFromFile("Data/Sprites/Battle/health_bar.png");
	buff.loadFromFile("Data/Sprites/Battle/buff.png");
	debuff.loadFromFile("Data/Sprites/Battle/debuff.png");
	confused.loadFromFile("Data/Sprites/Battle/confused.png");
	marked.loadFromFile("Data/Sprites/Battle/marked.png");
	sleeping.loadFromFile("Data/Sprites/Battle/sleeping.png");

	healthBarBigFrame.loadFromFile("Data/Sprites/Battle/health_bar_big_frame.png");
	healthBarBig.loadFromFile("Data/Sprites/Battle/health_bar_big.png");

	abilityAnnouncementBanner.loadFromFile("Data/Sprites/Battle/ability_announcement.png");

	abilityTargetMarker.loadFromFile("Data/Sprites/Battle/battle_marker.png");
	turnMarker.loadFromFile("Data/Sprites/Battle/turn_marker.png");

}


