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
	void GainHealth(int _health, bool _critical = false);

	void Mark(int _rounds) { marked += _rounds; }
	void Confuse(int _rounds) { confused += _rounds; }
	void PutToSleep() { sleeping = true; }
	void AddDecay(int _rounds, int _damage);
	void AddBounty(int _rounds) {bounty += _rounds;}
	void AddBountyBuff();
	void AddBuff(Buff _buff);
	void AddDebuff(Buff _buff);
	void SetFatigueLevel(FatigueLevel _level);
	void SetNofaceBuffLevel(int _level);
	void AddMiss(int _number = 1);
	void AddDynamite() {dynamite = true;}
	void RemoveDynamite() { dynamite = false; }
	void SetMissOnHighDamage(bool _missOnHighDamage) { missOnHighDamage = _missOnHighDamage; }

	void RemoveAllBuffs();
	void RemoveAllDebuffs();

	bool IsAsleep() { return sleeping; }
	bool SkipRound() { return skipRound; }
	bool IsMarked() { return marked > 0; }
	bool IsConfused() { return confused > 0; }
	bool IsBuffed() { return buffs.size() > 0; }
	bool IsDebuffed() { return debuffs.size() > 0; }
	bool HasBounty() { return bounty > 0; }
	int GetNofaceBuffLevel() { return nofaceBuffLevel; }
	void RemoveMiss();
	int NumberOfMisses();
	bool HasDynamite() { return dynamite; }
	CombatantAttributes GetNofaceStats();

	void CheckNofaceBuff();

	int RoundsConfused() { return confused; }
	int RoundsMarked() { return marked; }
	int RoundsDecay();
	int RoundsBounty() { return bounty; }

	Buff &GetBuff();
	Buff &GetDebuff();

	FatigueLevel GetFatigueLevel() { return fatigueLevel; }
	CombatantAttributes fatigueDebuff;
	CombatantAttributes nofaceBuff;

	int GetDamage();
	int GetDecay();
	int GetAttribute(const std::string &identifier);

	int *GetCurrentHealthPointer() { return &currentStats["currentHealth"]; }
	int *GetMaxHealthPointer() { return &currentStats["maxHealth"]; }

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
	int misses;
	int bounty;
	std::vector<std::pair<int, int>> decay;
	std::vector<Buff> buffs;
	std::vector<Buff> debuffs;
	FatigueLevel fatigueLevel;
	int nofaceBuffLevel;
	bool stupid;
	bool dynamite;
	bool missOnHighDamage;

	Buff returnBuff;

	void HandleBuffDurations(std::vector<Buff> &_buffs);
	void HandleDecay();
	void HandleDynamite();

	double statusAnnouncementTime;
	bool sleepChecked;
	bool decayChecked;
	bool dynamiteChecked;
	bool skipRound;
};