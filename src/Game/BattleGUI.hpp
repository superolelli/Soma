#pragma once

#include "GUI.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "../Framework/Gui/Bar.hpp"
#include "Resources\Resources.hpp"

class AdventureGroup;
class Combatant;

class BattleGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine) override;
	virtual void Update() override;
	virtual void Render() override;

	int GetCurrentAbility() { return currentAbility; }
	void SetCurrentCombatant(Combatant *_combatant);
	void SetAdventureGroup(AdventureGroup *_adventureGroup);

private:

	int currentAbility;
	Combatant *currentCombatant;
	AdventureGroup *players;

	CSprite abilities[4][4];

	CSprite currentAbilityFrame;
	Bar currentCombatantHealthBar;

	void ShowTooltip(int _ability);
	void ShowCombatantAttributes();
};