#pragma once
#include "Resources\Resources.hpp"
#include "AdventureGroup.hpp"
#include "BattleGUI.hpp"
#include "Enemy.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"
#include "LootableDialog.hpp"
#include "LevelStatus.hpp"

const int ENEMY_X_OFFSET = 20;
const int ENEMY_Y_POS = 750;
const int ENEMY_SPACING = 40;

class Battle
{
public:

	Battle(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, CombatantID enemyIDs[4], bool _boss, LevelStatus *_levelStatus);
	~Battle();

	void Update();
	void Render();
	void RenderAbilityAnimations();
	bool CurrentlyExecutingAbility();

	void AddEnemy(CombatantID enemyID);

	bool EnemyDiedLastRound();

	bool isFinished() { return isBattleFinished; }
	bool isBossBattle;

	std::array<Enemy*, 4> &GetEnemies() {return enemies;}
	LevelStatus* GetLevelStatus() { return levelStatus; }

private:

	AdventureGroup *players;
	BattleGUI *gui;
	CGameEngine *engine;
	NotificationRenderer *notificationRenderer;
	LevelStatus* levelStatus;

	std::array<Enemy*, 4> enemies;
	std::vector<Combatant*> combatants;

	int currentCombatant;

	int turnsSinceLastEnemyDied;

	bool isBattleFinished;

	void InitCombatants(int _xView, CombatantID enemyIDs[4]);
	
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

	Enemy *CreateEnemy(CombatantID _enemyID);
};