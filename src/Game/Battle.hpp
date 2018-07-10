#pragma once
#include "Resources\Resources.hpp"
#include "AdventureGroup.hpp"
#include "BattleGUI.hpp"
#include "Enemy.hpp"

enum abilityPhase {ready, aimed, finished};


const int ENEMY_X_OFFSET = 50;
const int ENEMY_Y_POS = 700;
const int ENEMY_SPACING = 200;

class Battle
{
public:

	void Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine);
	void Update();
	void Render();
	void Quit();

	bool isFinished() { return isBattleFinished; }

private:

	Enemy enemy[4];
	AdventureGroup *players;
	BattleGUI *gui;
	CGameEngine *engine;

	std::vector<Combatant*> combatants;
	int currentCombatant;
	int selectedTarget;

	float abilityAnnouncementTime;

	bool isBattleFinished;

	abilityPhase abilityStatus;


	void CalculateTurnOrder();
	void HandleDeaths();
	void ChooseNextCombatant();
	void DoCurrentAbility();
	void RenderAbilityAnnouncement();

	bool AimChosen();
	bool CombatantClicked(int _id);
	bool CurrentAbilityCanAimAtCombatant(int i);
	bool CheckIfOneGroupIsDead();
};