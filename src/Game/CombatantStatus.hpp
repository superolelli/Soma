#pragma once

#include "Buff.hpp"
#include <vector>
#include "../Framework/Graphics/NotificationRenderer.hpp"

class Combatant;

class CombatantStatus
{
public:

	enum class FatigueLevel { awake, tired, stupid };

	void Init(Combatant *_combatant, NotificationRenderer *_notificationRenderer);

	void UpdateStatusForNewTurn(double _initialWaitingTime = 0.0);
	void ExecuteStatusChanges();

	bool IsExecutingStatusChanges();

	void LooseHealth(int _damage, bool _critical, bool _useArmour = true);
	void GainHealth(int _health);

	void Mark(int _rounds) { marked += _rounds; }
	void Confuse(int _rounds) { confused += _rounds; }
	void PutToSleep() { sleeping = true; }
	void DoDamageOverTime(int _rounds, int _damage);
	void AddBuff(Buff _buff);
	void AddDebuff(Buff _buff);
	void SetFatigueLevel(FatigueLevel _level);

	void RemoveAllBuffs();
	void RemoveAllDebuffs();

	bool IsAsleep() { return sleeping; }
	bool SkipRound() { return skipRound; }
	bool IsMarked() { return marked > 0; }
	bool IsConfused() { return confused > 0; }
	bool IsBuffed() { return buffs.size() > 0; }
	bool IsDebuffed() { return debuffs.size() > 0; }

	int RoundsConfused() { return confused; }
	int RoundsMarked() { return marked; }

	int RoundsDamageOverTime();
	int DamageOverTime();

	Buff &GetBuff();
	Buff &GetDebuff();

	FatigueLevel GetFatigueLevel() { return fatigueLevel; }
	const CombatantAttributes fatigueDebuff{ 0,0,0,-3,-3,0,-5,-5,-1 };

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

	void SetStats(CombatantAttributes &_stats) { currentStats = _stats; }
	void AddStats(CombatantAttributes &_stats) { currentStats += _stats; }
	void Reset();

private:

	CombatantAttributes currentStats;
	Combatant *combatant;
	NotificationRenderer *notificationRenderer;

	int confused;
	bool sleeping;
	int marked;
	std::vector<std::pair<int, int>> damageOverTime;
	std::vector<Buff> buffs;
	std::vector<Buff> debuffs;
	FatigueLevel fatigueLevel;
	bool stupid;

	Buff returnBuff;

	void HandleBuffDurations(std::vector<Buff> &_buffs);
	void HandleDamageOverTime();

	double statusAnnouncementTime;
	bool sleepChecked;
	bool damageOverTimeChecked;
	bool skipRound;
};