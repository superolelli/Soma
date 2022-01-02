#pragma once

#include "GUI.hpp"
#include "AbilityTooltip.hpp"
#include "../Framework/Gui/Bar.hpp"
#include "Resources\Resources.hpp"
#include "CombatantAttributesPanel.hpp"
#include "CombatantResistancesPanel.hpp"
#include "LevelGUICommon.hpp"

class Combatant;
class GameStatus;

class BattleGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine, GameStatus *_gameStatus, NotificationRenderer *_notificationRenderer);
	virtual void Update() override;
	virtual void Render() override;

	int GetCurrentAbility() { return currentAbility; }
	bool ShouldSkipTurn() { return skipTurn; }
	void SetCurrentAbility(int _ability);
	void SetCombatantToDisplay(Combatant *_combatant);
	void SetCurrentPlayer(Combatant *_combatant);

private:
	CGameEngine *engine;

	LevelGUICommon commonGUIParts;

	int currentAbility;
	bool skipTurn;
	Combatant *combatantToDisplay;
	Combatant *currentPlayer;
	AdventureGroup *players;

	CSprite abilityPanel;
	CSprite abilities[4][4];
	CSprite currentAbilityFrame;
	CButton skipTurnButton;

	CSprite combatantInformationPanel;
	CombatantAttributesPanel combatantAttributesPanel;
	CombatantResistancesPanel combatantResistancesPanel;
	Bar currentCombatantHealthBar;

	sf::Text currentCombatantName;

	sf::Text abilityInformationText;
	AbilityTooltip tooltip;

	void ShowTooltip(int _ability);
	void RenderCombatantInformation();
	void UpdateAbilityInformationText();
};