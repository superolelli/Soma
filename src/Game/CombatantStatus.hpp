#pragma once

#include "Buff.hpp"
#include <vector>


class CombatantStatus
{
public:

	void HandleStatusChanges();

	void LooseHealth(int _damage);
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

	int GetMaxHealth();
	int GetCurrentHealth();
	int GetDamage();
	int GetArmour();
	int GetInitiative();

	int *GetCurrentHealthPointer() { return &attributes.currentHealth; }
	int *GetMaxHealthPointer() { return &attributes.maxHealth; }

	void SetAttributes(CombatantAttributes _attributes) { attributes = _attributes; }
	void Reset();

private:

	CombatantAttributes attributes;

	int confused;
	bool sleeping;
	int marked;
	std::vector<Buff> buffs;
	std::vector<Buff> debuffs;


	void HandleBuffDurations(std::vector<Buff> &_buffs);
};