#pragma once
#include "Resources\Resources.hpp"
#include "AdventureGroup.hpp"
#include "BattleGUI.hpp"
#include "Enemy.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"
#include "LootableDialog.hpp"

const int ENEMY_X_OFFSET = 20;
const int ENEMY_Y_POS = 750;
const int ENEMY_SPACING = 40;

class Battle
{
public:

	void Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, int enemyIDs[4], bool _boss);
	void Update();
	void Render();
	void RenderAbilityAnimations();
	bool CurrentlyExecutingAbility();
	void Quit();

	void AddEnemy(int enemyID);

	bool EnemyDiedLastRound();

	bool isFinished() { return isBattleFinished; }
	bool isBossBattle;

	std::array<Enemy*, 4> &GetEnemies() {return enemies;}

private:

	AdventureGroup *players;
	BattleGUI *gui;
	CGameEngine *engine;
	NotificationRenderer *notificationRenderer;


	std::array<Enemy*, 4> enemies;
	std::vector<Combatant*> combatants;

	int currentCombatant;

	int turnsSinceLastEnemyDied;

	bool isBattleFinished;

	void InitCombatants(int _xView, int enemyIDs[4]);
	
	void InitNewRound();
	void CalculateTurnOrder();
	void HandleDeaths();
	void ChooseNextCombatant();
	void GiveTurnToNextCombatant();

	void SetCombatantToDisplayForGUI();

	bool IsOneGroupDead();
	bool IsOneGroupDying();

	int GetEmptyEnemyBattlePosition();
	void RecalculateEnemyPositions();

	Enemy *CreateEnemy(int _enemyID);
};