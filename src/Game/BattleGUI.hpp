#pragma once

#include "GUI.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "Resources\Resources.hpp"
#include "CombatantID.hpp"

class AdventureGroup;

class BattleGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine) override;
	virtual void Update() override;
	virtual void Render() override;

	int GetCurrentAbility() { return currentAbility; }
	void SetCurrentPlayer(int _player);
	void SetAdventureGroup(AdventureGroup *_adventureGroup);

private:

	int currentAbility;
	CombatantID currentPlayer;
	AdventureGroup *players;

	CSprite abilities[4][4];

	CSprite currentAbilityFrame;

	void ShowTooltip(int _ability);
	void ShowPlayerAttributes();
};