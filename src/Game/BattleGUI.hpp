#pragma once

#include "GUI.hpp"
#include "../Framework/Graphics/Sprite.hpp"
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

	void ShowTooltip(int _ability);
	void RenderCombatantInformation();
	void GenerateTooltipString(std::string &_tooltip, int _ability);
	void AppendTooltipStringForOneTarget(std::string &_tooltip, int _ability, bool _hostileAbility, bool _indent);

	Ability &GetAbility(int _ability);
};