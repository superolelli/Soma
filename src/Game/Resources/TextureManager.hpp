#pragma once

#include <SFML/Graphics.hpp>
#include "../../Framework/Patterns/singleton.hpp"


#define g_pTextures CTextureManager::Get()
class CTextureManager : public TSingleton<CTextureManager>
{
public:

	void LoadTextures();

	sf::Image cursorNormal;
	sf::Image cursorHand;

	sf::Texture statusBar;

	sf::Texture levelFinishedPanel[2];
	sf::Texture levelFinishedButton;
	sf::Texture bangGenericButton;
	sf::Texture confirmationDialogPanel;

	/** MAIN MENU **/
	sf::Texture mainMenuBackground;
	sf::Texture savegamePanel;
	sf::Texture nameInputBackground;


	/** BACKGROUNDS **/
	std::vector<sf::Texture> firstLayerBackgrounds[2];
	std::vector<sf::Texture> secondLayerBackgrounds[2];
	std::vector<sf::Texture> thirdLayerBackgrounds[2];
	sf::Texture fourthLayerBackground[2];
	sf::Texture bangDoor;

	sf::Texture mainRoomBackgrounds[4];
	sf::Texture mainRoomDoors[3];
	sf::Texture mainRoomSigns[3];
	sf::Texture mainRoomRoots;
	sf::Texture mainRoomVendingMachine;

	/** MAIN ROOM BUTTONS **/
	sf::Texture mainRoomButtonInventory;
	sf::Texture mainRoomButtonSkillPanel;
	sf::Texture mainRoomButtonShop;

	/** LOOTABLES **/
	sf::Texture lootable[17];

	sf::Texture speechBubble;

	/** ITEMS & INVENTORY **/
	sf::Texture inventoryPanel;
	sf::Texture scrollableItemPanel;
	sf::Texture inventoryDiamond;
	sf::Texture inventoryConnections;
	sf::Texture inventoryConnection[4];
	sf::Texture inventoryConnectionsBackground;
	sf::Texture inventoryEquipmentField;
	sf::Texture inventoryScrollBar;
	sf::Texture lootablePanel;
	sf::Texture lootablePanelButton;
	sf::Texture lootablePanelCloseButton;
	sf::Texture consumablePanel;
	sf::Texture itemRowPanel;
	sf::Texture sortColorButton;
	sf::Texture sortNamesButton;
	sf::Texture item[35];

	/** VENDING MACHINE **/
	sf::Texture vendingMachinePanel;
	sf::Texture shopPanel;
	sf::Texture selectedItemFrame;
	sf::Texture priceSign;
	sf::Texture cardsSymbol;
	sf::Texture sellMultiplePanel;
	sf::Texture sellMultipleSlider;
	sf::Texture sellMultipleKnob;
	sf::Texture shopSlotLock;
	sf::Texture priceLock;

	/** SKILLING **/
	sf::Texture skills[4][4][8];
	sf::Texture skillPanelSelectedSkillFrame;
	sf::Texture skillPanel;
	sf::Texture skillPanelBridgePiece;
	sf::Texture skillPanelAbilityPlaceholder;
	sf::Texture skillPanelSkilledIndicator;
	sf::Texture skillPanelConnectionsSkilled[4];
	sf::Texture skillPanelConnectionsNotSkilled[4];
	sf::Texture skillPanelDice;
	sf::Texture skillPanelButtonNext;
	sf::Texture skillPanelButtonPrevious;
	sf::Texture skillPanelButtonClose;

	/** BATTLE GUI **/
	sf::Texture abilities[4][4];
	sf::Texture currentAbilityFrame;
	sf::Texture abilityPanel;
	sf::Texture skipTurnButton;

	sf::Texture combatantInformationPanel;
	sf::Texture healthBarBigFrame;
	sf::Texture healthBarBig;

	/** LEVEL GUI **/
	sf::Texture fatigueBar;
	sf::Texture fatigueBarFrame;

	/** BATTLE **/
	sf::Texture healthBarFrame;
	sf::Texture healthBar;
	sf::Texture buff;
	sf::Texture debuff;
	sf::Texture confused;
	sf::Texture marked;
	sf::Texture sleeping;
	sf::Texture fistOfRevengeDebuff;
	sf::Texture decay;
	sf::Texture fatigueTired;
	sf::Texture fatigueStupid;
	sf::Texture nofaceBuff;
	sf::Texture missed;
	sf::Texture dynamite;
	sf::Texture bounty;

	sf::Texture abilityAnnouncementBanner;
	sf::Texture abilityTargetMarker;
	sf::Texture turnMarker;
	sf::Texture turnPendingMarker;

	/** POSSIBLE TARGETS **/
	sf::Texture possibleTargetSimon;
	sf::Texture possibleTargetOle;
	sf::Texture possibleTargetAnna;
	sf::Texture possibleTargetMarkus;
	sf::Texture possibleTargetEnemy;

private:

	void LoadSkillTextures(const std::string &_path, int playerID, int abilityID);
};