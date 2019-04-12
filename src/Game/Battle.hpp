#pragma once
#include "Resources\Resources.hpp"
#include "AdventureGroup.hpp"
#include "BattleGUI.hpp"
#include "Enemy.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"


const int ENEMY_X_OFFSET = 20;
const int ENEMY_Y_POS = 700;
const int ENEMY_SPACING = 40;

class Battle
{
public:

	void Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, int enemyIDs[4]);
	void Update();
	void Render();
	void RenderAbilityAnimations();
	void Quit();

	bool isFinished() { return isBattleFinished; }

private:

	Enemy enemy[4];
	AdventureGroup *players;
	BattleGUI *gui;
	CGameEngine *engine;

	std::vector<Combatant*> combatants;

	int currentCombatant;

	bool isBattleFinished;

	void CalculateTurnOrder();
	void HandleDeaths();
	void ChooseNextCombatant();

	bool IsOneGroupDead();
};