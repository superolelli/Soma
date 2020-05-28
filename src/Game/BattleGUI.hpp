#pragma once

#include "GUI.hpp"
#include "AbilityTooltip.hpp"
#include "../Framework/Gui/Bar.hpp"
#include "Resources\Resources.hpp"
#include "CombatantAttributesPanel.hpp"
#include "LevelGUICommon.hpp"

class Combatant;
class GameStatus;

class BattleGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine, GameStatus *_gameStatus);
	virtual void Update() override;
	virtual void Render() override;

	int GetCurrentAbility() { return currentAbility; }
	void SetCurrentAbility(int _ability);
	void SetCombatantToDisplay(Combatant *_combatant);
	void SetCurrentPlayer(Combatant *_combatant);

private:
	CGameEngine *engine;

	LevelGUICommon commonGUIParts;

	int currentAbility;
	Combatant *combatantToDisplay;
	Combatant *currentPlayer;
	AdventureGroup *players;

	CSprite abilityPanel;
	CSprite abilities[4][4];
	CSprite currentAbilityFrame;

	CSprite combatantInformationPanel;
	CombatantAttributesPanel combatantAttributesPanel;
	Bar currentCombatantHealthBar;

	sf::Text currentCombatantName;

	sf::Text abilityInformationText;
	AbilityTooltip tooltip;

	void ShowTooltip(int _ability);
	void RenderCombatantInformation();
	void UpdateAbilityInformationText();
};