#pragma once
#include "Resources\Resources.hpp"
#include "AdventureGroup.hpp"
#include "BattleGUI.hpp"
#include "Enemy.hpp"
#include "../Framework/Graphics/NotificationRenderer.hpp"
#include "../Framework/Patterns/Subject.hpp"


const int ENEMY_X_OFFSET = 20;
const int ENEMY_Y_POS = 700;
const int ENEMY_SPACING = 40;

class Battle : public CSubject
{
public:

	void Init(int _xView, AdventureGroup *_adventureGroup, BattleGUI *_gui, CGameEngine *_engine, NotificationRenderer *_notificationRenderer, int enemyIDs[4], bool _boss);
	void Update();
	void Render();
	void RenderAbilityAnimations();
	void Quit();

	bool isFinished() { return isBattleFinished; }

private:

	Enemy *enemy[4];
	AdventureGroup *players;
	BattleGUI *gui;
	CGameEngine *engine;

	std::vector<Combatant*> combatants;

	int currentCombatant;

	bool isBattleFinished;
	bool isPlayingIntro;
	bool isBossBattle;
	int finishedCycles;
	float afterIntroWaitingTime;

	void HandleIntro();
	
	void CalculateTurnOrder();
	void HandleDeaths();
	void ChooseNextCombatant();

	void SetCombatantToDisplayForGUI();

	bool IsOneGroupDead();
};