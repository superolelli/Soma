#include "TextureManager.hpp"



void CTextureManager::LoadTextures()
{
	statusBar.loadFromFile("Data/Sprites/Panels & Buttons/status_bar.png");

	bangLevelFinishedPanel.loadFromFile("Data/Sprites/Panels & Buttons/Bang/level_finished.png");
	bangLevelFailedPanel.loadFromFile("Data/Sprites/Panels & Buttons/Bang/level_failed.png");
	bangGenericButton.loadFromFile("Data/Sprites/Panels & Buttons/Bang/generic_button.png");

	/** BACKGROUNDS **/
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


	/** SKILLING **/
	LoadSkillTextures("Data/Sprites/Abilities/Simon/marshmallow_flash/marshmallow_flash_skill", 0, 0);
	LoadSkillTextures("Data/Sprites/Abilities/Simon/mess_around/mess_around_skill", 0, 1);
	LoadSkillTextures("Data/Sprites/Abilities/Simon/repair/repair_skill", 0, 2);
	LoadSkillTextures("Data/Sprites/Abilities/Simon/all_on_one/all_on_one_skill", 0, 3);

	LoadSkillTextures("Data/Sprites/Abilities/Ole/gummi_arm/gummi_arm_skill", 1, 0);
	LoadSkillTextures("Data/Sprites/Abilities/Ole/whole_body_kick/whole_body_kick_skill", 1, 1);
	LoadSkillTextures("Data/Sprites/Abilities/Ole/hack_into_the_system/hack_into_the_system_skill", 1, 2);
	LoadSkillTextures("Data/Sprites/Abilities/Ole/poetry_slam/poetry_slam_skill", 1, 3);

	LoadSkillTextures("Data/Sprites/Abilities/Anna/indomitable_happiness/indomitable_happiness_skill", 2, 0);
	LoadSkillTextures("Data/Sprites/Abilities/Anna/word_gush/word_gush_skill", 2, 1);
	LoadSkillTextures("Data/Sprites/Abilities/Anna/side_of_nose_blow/side_of_nose_blow_skill", 2, 2);
	LoadSkillTextures("Data/Sprites/Abilities/Anna/deathhug/deathhug_skill", 2, 3);

	LoadSkillTextures("Data/Sprites/Abilities/Markus/fist_of_revenge/fist_of_revenge_skill", 3, 0);
	LoadSkillTextures("Data/Sprites/Abilities/Markus/nose_ram/nose_ram_skill", 3, 1);
	LoadSkillTextures("Data/Sprites/Abilities/Markus/hard_destruction/hard_destruction_skill", 3, 2);
	LoadSkillTextures("Data/Sprites/Abilities/Markus/horrible_grin/horrible_grin_skill", 3, 3);

	skillPanelSelectedSkillFrame.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/selected_skill_frame.png");

	skillPanel.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/skill_panel.png");
	skillPanelBridgePiece.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/bridge_piece.png");
	skillPanelAbilityPlaceholder.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/ability_placeholders.png");
	skillPanelSkilledIndicator.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/skilled_indicator.png");

	skillPanelConnectionsSkilled[0].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_0_skilled.png");
	skillPanelConnectionsSkilled[1].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_1_skilled.png");
	skillPanelConnectionsSkilled[2].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_2_skilled.png");
	skillPanelConnectionsSkilled[3].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_4_skilled.png");

	skillPanelConnectionsNotSkilled[0].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_0_not_skilled.png");
	skillPanelConnectionsNotSkilled[1].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_1_not_skilled.png");
	skillPanelConnectionsNotSkilled[2].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_2_not_skilled.png");
	skillPanelConnectionsNotSkilled[3].loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/connection_4_not_skilled.png");

	skillPanelDice.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/dice.png");

	skillPanelButtonNext.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/button_next.png");
	skillPanelButtonPrevious.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/button_previous.png");
	skillPanelButtonClose.loadFromFile("Data/Sprites/Panels & Buttons/Skill_Panel/close_button.png");

	/** BATTLE GUI **/
	abilities[0][0].loadFromFile("Data/Sprites/Abilities/Simon/marshmallow_flash/marshmallow_flash.png");
	abilities[0][3].loadFromFile("Data/Sprites/Abilities/Simon/all_on_one/all_on_one.png");
	abilities[0][2].loadFromFile("Data/Sprites/Abilities/Simon/repair/repair.png");
	abilities[0][1].loadFromFile("Data/Sprites/Abilities/Simon/mess_around/mess_around.png");

	abilities[1][0].loadFromFile("Data/Sprites/Abilities/Ole/gummi_arm/gummi_arm.png");
	abilities[1][1].loadFromFile("Data/Sprites/Abilities/Ole/whole_body_kick/whole_body_kick.png");
	abilities[1][2].loadFromFile("Data/Sprites/Abilities/Ole/hack_into_the_system/hack_into_the_system.png");
	abilities[1][3].loadFromFile("Data/Sprites/Abilities/Ole/poetry_slam/poetry_slam.png");

	abilities[2][0].loadFromFile("Data/Sprites/Abilities/Anna/indomitable_happiness/indomitable_happiness.png");
	abilities[2][1].loadFromFile("Data/Sprites/Abilities/Anna/word_gush/word_gush.png");
	abilities[2][2].loadFromFile("Data/Sprites/Abilities/Anna/side_of_nose_blow/side_of_nose_blow.png");
	abilities[2][3].loadFromFile("Data/Sprites/Abilities/Anna/deathhug/deathhug.png");

	abilities[3][0].loadFromFile("Data/Sprites/Abilities/Markus/fist_of_revenge/fist_of_revenge.png");
	abilities[3][1].loadFromFile("Data/Sprites/Abilities/Markus/nose_ram/nose_ram.png");
	abilities[3][2].loadFromFile("Data/Sprites/Abilities/Markus/hard_destruction/hard_destruction.png");
	abilities[3][3].loadFromFile("Data/Sprites/Abilities/Markus/horrible_grin/horrible_grin.png");

	currentAbilityFrame.loadFromFile("Data/Sprites/Abilities/currentAbilityFrame.png");
	abilityPanel.loadFromFile("Data/Sprites/Abilities/ability_panel.png");

	healthBarBigFrame.loadFromFile("Data/Sprites/Battle/health_bar_big_frame.png");
	healthBarBig.loadFromFile("Data/Sprites/Battle/health_bar_big.png");

	combatantInformationPanel.loadFromFile("Data/Sprites/Battle/combatant_information_panel.png");

	/** BATTLE **/
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


void CTextureManager::LoadSkillTextures(const std::string &_path, int playerID, int abilityID)
{
	for (int i = 0; i < 8; i++)
	{
		skills[playerID][abilityID][i].loadFromFile(_path + "_" + std::to_string(i + 1) + ".png");
	}
}