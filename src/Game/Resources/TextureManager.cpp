#include "TextureManager.hpp"



void CTextureManager::LoadTextures()
{
	statusBar.loadFromFile("Data/Sprites/Panels & Buttons/status_bar.png");

	bangLevelFinishedPanel.loadFromFile("Data/Sprites/Panels & Buttons/Bang/level_finished.png");
	bangLevelFailedPanel.loadFromFile("Data/Sprites/Panels & Buttons/Bang/level_failed.png");
	bangGenericButton.loadFromFile("Data/Sprites/Panels & Buttons/Bang/generic_button.png");

	/** BACKGROUNDS **/
	bangFirstLayerBackgrounds[room_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_raum1.png");
	bangFirstLayerBackgrounds[room_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_raum2.png");
	bangFirstLayerBackgrounds[room_3].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_raum3.png");
	bangFirstLayerBackgrounds[corridor_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_1.png");
	bangFirstLayerBackgrounds[corridor_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_2.png");
	bangFirstLayerBackgrounds[corridor_3].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_3.png");
	bangFirstLayerBackgrounds[corridor_4].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_4.png");
	bangFirstLayerBackgrounds[corridor_5].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_5.png");
	bangFirstLayerBackgrounds[corridor_6].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_6.png");
	bangFirstLayerBackgrounds[corridor_7].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_7.png");
	bangFirstLayerBackgrounds[corridor_8].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_8.png");
	bangFirstLayerBackgrounds[door_1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_t�r_1.png");
	bangFirstLayerBackgrounds[door_2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_t�r_2.png");
	bangFirstLayerBackgrounds[end].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_ende.png");

	bangSecondLayerBackgrounds[0].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_second_layer_1.png");
	bangSecondLayerBackgrounds[1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_second_layer_2.png");
	bangSecondLayerBackgrounds[2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_second_layer_3.png");
	bangSecondLayerBackgrounds[3].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_second_layer_4.png");
	bangSecondLayerBackgrounds[4].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_second_layer_5.png");
	bangSecondLayerBackgrounds[5].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_second_layer_6.png");
	bangThirdLayerBackgrounds[0].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_third_layer_1.png");
	bangThirdLayerBackgrounds[1].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_third_layer_2.png");
	bangThirdLayerBackgrounds[2].loadFromFile("Data/Sprites/Backgrounds/Bang/bang_third_layer_3.png");
	bangFourthLayerBackground.loadFromFile("Data/Sprites/Backgrounds/Bang/bang_fourth_layer.png");

	bangDoor.loadFromFile("Data/Sprites/Backgrounds/Bang/bang_gang_t�r.png");

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
	mainRoomVendingMachine.loadFromFile("Data/Sprites/Backgrounds/Main Room/vending_machine.png");

	/** MAIN ROOM BUTTONS **/
	mainRoomButtonInventory.loadFromFile("Data/Sprites/Panels & Buttons/inventory_button.png");
	mainRoomButtonSkillPanel.loadFromFile("Data/Sprites/Panels & Buttons/skillpanel_button.png");
	mainRoomButtonShop.loadFromFile("Data/Sprites/Panels & Buttons/shop_button.png");

	/** LOOTABLES **/
	lootable[0].loadFromFile("Data/Sprites/Lootables/barrel.png");
	lootable[1].loadFromFile("Data/Sprites/Lootables/cabinet.png");
	lootable[2].loadFromFile("Data/Sprites/Lootables/chest.png");
	lootable[3].loadFromFile("Data/Sprites/Lootables/cup.png");
	lootable[4].loadFromFile("Data/Sprites/Lootables/keg.png");
	lootable[5].loadFromFile("Data/Sprites/Lootables/rubbish.png");
	lootable[6].loadFromFile("Data/Sprites/Lootables/sack.png");
	lootable[7].loadFromFile("Data/Sprites/Lootables/skeleton.png");
	lootable[8].loadFromFile("Data/Sprites/Lootables/stonepile.png");
	lootable[9].loadFromFile("Data/Sprites/Lootables/table.png");
	lootable[10].loadFromFile("Data/Sprites/Lootables/wagon.png");

	/** ITEMS **/
	inventoryPanel.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/inventory_panel.png");
	scrollableItemPanel.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/scrollable_item_panel.png");
	inventoryDiamond.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/diamond.png");
	inventoryConnections.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/equipment_connections.png");
	inventoryConnection[2].loadFromFile("Data/Sprites/Panels & Buttons/Inventory/connection_upper.png");
	inventoryConnection[1].loadFromFile("Data/Sprites/Panels & Buttons/Inventory/connection_right.png");
	inventoryConnection[0].loadFromFile("Data/Sprites/Panels & Buttons/Inventory/connection_lower.png");
	inventoryConnection[3].loadFromFile("Data/Sprites/Panels & Buttons/Inventory/connection_left.png");
	inventoryConnectionsBackground.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/equipment_connection_background.png");
	inventoryEquipmentField.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/equipment_field.png");
	inventoryScrollBar.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/scroll_bar.png");
	lootablePanel.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/lootable_panel.png");
	lootablePanelButton.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/take_all_button.png");
	lootablePanelCloseButton.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/lootable_panel_close_button.png");
	consumablePanel.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/consumables_panel.png");
	itemRowPanel.loadFromFile("Data/Sprites/Panels & Buttons/Inventory/item_row_panel.png");
	sortColorButton.loadFromFile("Data/Sprites/Panels & Buttons/sort_color_button.png");
	sortNamesButton.loadFromFile("Data/Sprites/Panels & Buttons/sort_names_button.png");
	item[0].loadFromFile("Data/Sprites/Items/Bang/cards.png");
	item[1].loadFromFile("Data/Sprites/Items/Bang/dice.png");
	item[2].loadFromFile("Data/Sprites/Items/Bang/iron_plate.png");
	item[3].loadFromFile("Data/Sprites/Items/Bang/sombrero.png");
	item[4].loadFromFile("Data/Sprites/Items/Bang/binocular.png");
    item[5].loadFromFile("Data/Sprites/Items/Bang/bible.png");
    item[6].loadFromFile("Data/Sprites/Items/Bang/cowboyhat.png");
    item[7].loadFromFile("Data/Sprites/Items/Bang/lasso.png");
	item[8].loadFromFile("Data/Sprites/Items/Bang/ammo.png");
	item[9].loadFromFile("Data/Sprites/Items/Bang/knife.png");
	item[10].loadFromFile("Data/Sprites/Items/Bang/tomahawk.png");
	item[11].loadFromFile("Data/Sprites/Items/Bang/sherrif_star.png");
	item[12].loadFromFile("Data/Sprites/Items/Bang/boot.png");
	item[13].loadFromFile("Data/Sprites/Items/Bang/horseshoe.png");
	item[14].loadFromFile("Data/Sprites/Items/Bang/canteen.png");
	item[15].loadFromFile("Data/Sprites/Items/Bang/pepperbox.png");
	item[16].loadFromFile("Data/Sprites/Items/Bang/scope.png");
	item[17].loadFromFile("Data/Sprites/Items/Bang/beer.png");
	item[18].loadFromFile("Data/Sprites/Items/Bang/tequila.png");
	item[19].loadFromFile("Data/Sprites/Items/Bang/whisky.png");

	/** VENDING MACHINE **/
	vendingMachinePanel.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/vending_machine_panel.png");
	shopPanel.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/shop_panel.png");
	selectedItemFrame.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/selected_item_frame.png");
	priceSign.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/price_sign.png");
	cardsSymbol.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/cards_symbol.png");
	sellMultiplePanel.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/sell_multiple_dialog.png");
	sellMultipleSlider.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/slider_bar.png");
	sellMultipleKnob.loadFromFile("Data/Sprites/Panels & Buttons/VendingMachinePanel/slider_knob.png");

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

	/** LEVEL GUI **/
	fatigueBar.loadFromFile("Data/Sprites/Panels & Buttons/fatigue_bar.png");
	fatigueBarFrame.loadFromFile("Data/Sprites/Panels & Buttons/fatigue_bar_frame.png");

	/** BATTLE **/
	healthBarFrame.loadFromFile("Data/Sprites/Battle/health_bar_frame.png");
	healthBar.loadFromFile("Data/Sprites/Battle/health_bar.png");
	buff.loadFromFile("Data/Sprites/Battle/buff.png");
	debuff.loadFromFile("Data/Sprites/Battle/debuff.png");
	confused.loadFromFile("Data/Sprites/Battle/confused.png");
	marked.loadFromFile("Data/Sprites/Battle/marked.png");
	sleeping.loadFromFile("Data/Sprites/Battle/sleeping.png");
	damageOverTime.loadFromFile("Data/Sprites/Battle/damage_over_time.png");
	fatigueTired.loadFromFile("Data/Sprites/Battle/fatigue_debuff.png");
	fatigueStupid.loadFromFile("Data/Sprites/Battle/fatigue_debuff2.png");

	abilityAnnouncementBanner.loadFromFile("Data/Sprites/Battle/ability_announcement.png");

	abilityTargetMarker.loadFromFile("Data/Sprites/Battle/battle_marker.png");
	turnMarker.loadFromFile("Data/Sprites/Battle/turn_marker.png");
	turnPendingMarker.loadFromFile("Data/Sprites/Battle/pending_turn_marker.png");

	possibleTargetSimon.loadFromFile("Data/Sprites/Abilities/PossibleTargets/target_simon.png");
	possibleTargetOle.loadFromFile("Data/Sprites/Abilities/PossibleTargets/target_ole.png");
	possibleTargetAnna.loadFromFile("Data/Sprites/Abilities/PossibleTargets/target_anna.png");
	possibleTargetMarkus.loadFromFile("Data/Sprites/Abilities/PossibleTargets/target_markus.png");
	possibleTargetEnemy.loadFromFile("Data/Sprites/Abilities/PossibleTargets/target_enemy.png");

}


void CTextureManager::LoadSkillTextures(const std::string &_path, int playerID, int abilityID)
{
	for (int i = 0; i < 8; i++)
	{
		skills[playerID][abilityID][i].loadFromFile(_path + "_" + std::to_string(i + 1) + ".png");
	}
}