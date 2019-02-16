#pragma once

#include "Buff.hpp"
#include <vector>
#include "../Framework/Graphics/NotificationRenderer.hpp"

class Combatant;

class CombatantStatus
{
public:

	void Init(Combatant *_combatant, NotificationRenderer *_notificationRenderer);

	void HandleStatusChanges();

	void LooseHealth(int _damage, bool _critical);
	void GainHealth(int _health);

	void Mark(int _rounds) { marked += _rounds; }
	void Confuse(int _rounds) { confused += _rounds; }
	void PutToSleep() { sleeping = true; }
	void AddBuff(Buff _buff);
	void AddDebuff(Buff _buff);

	void RemoveAllBuffs();
	void RemoveAllDebuffs();

	bool IsAsleep() { return sleeping; }
	bool IsMarked() { return marked > 0; }
	bool IsConfused() { return confused > 0; }
	bool IsBuffed() { return buffs.size() > 0; }
	bool IsDebuffed() { return debuffs.size() > 0; }

	int RoundsConfused() { return confused; }

	Buff &GetBuff();
	Buff &GetDebuff();

	int GetMaxHealth();
	int GetCurrentHealth();
	int GetDamage();
	int GetDamageMin();
	int GetDamageMax();
	int GetArmour();
	int GetCriticalHit();
	int GetDodge();
	int GetPrecision();
	int GetInitiative();

	int *GetCurrentHealthPointer() { return &currentStats.currentHealth; }
	int *GetMaxHealthPointer() { return &currentStats.maxHealth; }

	void SetStats(CombatantStats _stats) { currentStats = _stats; }
	void Reset();

private:

	CombatantStats currentStats;
	Combatant *combatant;
	NotificationRenderer *notificationRenderer;

	int confused;
	bool sleeping;
	int marked;
	std::vector<Buff> buffs;
	std::vector<Buff> debuffs;

	Buff returnBuff;

	void HandleBuffDurations(std::vector<Buff> &_buffs);
};