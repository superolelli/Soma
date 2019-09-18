#pragma once

#include "GUI.hpp"
#include "AbilityTooltip.hpp"
#include "../Framework/Gui/Bar.hpp"
#include "Resources\Resources.hpp"
#include "CombatantAttributesPanel.hpp"

class AdventureGroup;
class Combatant;

class BattleGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine) override;
	virtual void Update() override;
	virtual void Render() override;

	int GetCurrentAbility() { return currentAbility; }
	void SetCombatantToDisplay(Combatant *_combatant);
	void SetCurrentPlayer(Combatant *_combatant);
	void SetAdventureGroup(AdventureGroup *_adventureGroup);

private:

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
	
	AbilityTooltip tooltip;

	void ShowTooltip(int _ability);
	void RenderCombatantInformation();
};