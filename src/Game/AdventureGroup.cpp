#include "AdventureGroup.hpp"
#include "Markus.hpp"

AdventureGroup::AdventureGroup(CGameEngine* _engine, NotificationRenderer* _notificationRenderer)
{
	adventurer[CombatantID::Ole] = new Player(CombatantID::Ole, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID::Ole));
	adventurer[CombatantID::Anna] = new Player(CombatantID::Anna, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID::Anna));
	adventurer[CombatantID::Simon] = new Player(CombatantID::Simon, _engine, _notificationRenderer, g_pModels->GetNewCombatantModelInstance(CombatantID::Simon));
	adventurer[CombatantID::Markus] = new PlayerMarkus(_engine, _notificationRenderer);

	int x = GROUP_OFFSET_LEFT;
	for (int i = 0; i < 4; i++)
	{
		adventurer[i]->SetPos(x, GROUP_Y_POS);
		adventurer[i]->SetEquipment(g_pGameStatus->GetEquipmentStats(i).stats);
		adventurer[i]->SetEquipment(g_pGameStatus->GetDiamondStats(i));
		adventurer[i]->Status().SetMissOnHighDamage(g_pGameStatus->GetEquipmentStats(i).missOnHighDamage);
		adventurer[i]->Status().SetHealOnPass(g_pGameStatus->GetEquipmentStats(i).healOnPass);
		x += PLAYER_SPACING;
	}
}


AdventureGroup::~AdventureGroup()
{
	for (auto &a : adventurer)
		SAFE_DELETE(a);
}

void AdventureGroup::Update(int xMove)
{
	for (auto a : adventurer)
	{
		a->Update(xMove, xMove != 0);
	}
	
}


void AdventureGroup::Render()
{
	for (auto a : adventurer)
	{
		if(a->Status().GetAttribute("currentHealth") > 0)
			a->Render();
	}

	for (auto a : adventurer)
	{
		if (a->Status().GetAttribute("currentHealth") > 0)
			a->RenderStatusBar();
	}
}

void AdventureGroup::SetFatigueLevelOfAllPlayers(CombatantStatus::FatigueLevel _level)
{
	for (auto& a : adventurer)
		a->Status().SetFatigueLevel(_level);
}


bool AdventureGroup::IsDead()
{
	for (auto &a : adventurer)
	{
		if (a->Status().GetAttribute("currentHealth") > 0)
			return false;
	}

	return true;
}
