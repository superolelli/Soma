#include "CombatantStatus.hpp"
#include "Resources\FontManager.hpp"
#include "Combatant.hpp"



void CombatantStatus::Init(Combatant *_combatant, NotificationRenderer *_notificationRenderer)
{
	combatant = _combatant;
	notificationRenderer = _notificationRenderer;
}


void CombatantStatus::UpdateStatusForNewTurn(double _initialWaitingTime)
{
	sleepChecked = false;
	damageOverTimeChecked = false;
	skipRound = false;
	statusAnnouncementTime = _initialWaitingTime;

	if (marked > 0)
		marked--;

	if (confused > 0)
		confused--;

	HandleBuffDurations(buffs);
	HandleBuffDurations(debuffs);
}


void CombatantStatus::ExecuteStatusChanges()
{
	if (statusAnnouncementTime > 0.0)
	{
		statusAnnouncementTime -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
		return;
	}

	if (!damageOverTimeChecked)
	{
		HandleDamageOverTime();
		damageOverTimeChecked = true;
	}
	else if (!sleepChecked)
	{
		sleepChecked = true;
		if (sleeping == true)
		{
			sleeping = false;
			skipRound = true;
			combatant->SetAnimation("idle", IDLE_ANIMATION_SPEED);
			statusAnnouncementTime = 2.0;
			notificationRenderer->AddNotification("Aufgewacht!", g_pFonts->f_kingArthur, sf::Vector2f(combatant->GetRect().left - combatant->GetRect().width / 2.0f, combatant->GetRect().top - 20.0f), 1.0f);
		}
	}

}

bool CombatantStatus::IsExecutingStatusChanges()
{
	return !sleepChecked || !damageOverTimeChecked || statusAnnouncementTime > 0.0;
}



void CombatantStatus::HandleBuffDurations(std::vector<Buff> &_buffs)
{
	std::vector<Buff>::iterator i;

	for (i = _buffs.begin(); i != _buffs.end();)
	{
		if (i->duration > 0)
			i->duration--;

		if (i->duration <= 0)
		{
			currentStats -= i->stats;
			i = _buffs.erase(i);
		}
		else
			i++;
	}
}

void CombatantStatus::HandleDamageOverTime()
{
	int damage = 0;
	std::vector<std::pair<int, int>>::iterator i;
	for (i = damageOverTime.begin(); i != damageOverTime.end();)
	{
		if (i->first > 0)
		{
			i->first--;
			damage += i->second;
		}

		if (i->first <= 0)
			i = damageOverTime.erase(i);
		else
			i++;
	}

	if (damage > 0)
	{
		LooseHealth(damage, false, false);
		statusAnnouncementTime = 2.0;
		g_pSounds->PlaySound(DAMAGE_OVER_TIME);

		if (GetCurrentHealth() <= 0)
			skipRound = true;
	}
}


void CombatantStatus::LooseHealth(int _damage, bool _critical, bool _useArmour)
{
	int damage = _damage;

	if(_useArmour)
		damage -= std::round(((float)currentStats.armour / 100.0f * _damage));

	currentStats.currentHealth -= damage;

	if (currentStats.currentHealth < 0)
		currentStats.currentHealth = 0;

	auto notificationPos = sf::Vector2f(combatant->GetRect().left + combatant->GetRect().width / 2.0f, combatant->GetRect().top);

	if(_critical)
		notificationRenderer->AddNotification(std::to_string(damage), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color(180, 0, 0), sf::Color::Black, 40);
	else
		notificationRenderer->AddNotification(std::to_string(damage), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color::Red, sf::Color::Black);
}


void CombatantStatus::GainHealth(int _health)
{
	currentStats.currentHealth += _health;

	if (currentStats.currentHealth > currentStats.maxHealth)
		currentStats.currentHealth = currentStats.maxHealth;

	auto notificationPos = sf::Vector2f(combatant->GetRect().left + combatant->GetRect().width / 2.0f, combatant->GetRect().top);
	notificationRenderer->AddNotification(std::to_string(_health), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color::Green, sf::Color::Black);
}

void CombatantStatus::CheckNofaceBuff()
{
	if (nofaceBuffLevel >= 0)
	{
		SetNofaceBuffLevel((currentStats.maxHealth - currentStats.currentHealth) / 5);
	}
}


void CombatantStatus::DoDamageOverTime(int _rounds, int _damage)
{
	damageOverTime.push_back(std::pair<int, int>(_rounds, _damage));
}

void CombatantStatus::AddBuff(Buff _buff)
{
	currentStats += _buff.stats;
	buffs.push_back(_buff);
}


void CombatantStatus::AddDebuff(Buff _buff)
{
	currentStats += _buff.stats;
	debuffs.push_back(_buff);
}

void CombatantStatus::SetFatigueLevel(FatigueLevel _level)
{
	if (fatigueLevel == _level)
		return;

	if (fatigueLevel == FatigueLevel::tired)
		currentStats -= fatigueDebuff;
	else if (fatigueLevel == FatigueLevel::stupid)
		stupid = false;

	if (_level == FatigueLevel::tired)
		currentStats += fatigueDebuff;
	else if (_level == FatigueLevel::stupid)
		stupid = true;

	fatigueLevel = _level;
}

void CombatantStatus::SetNofaceBuffLevel(int _level)
{
	if (_level == nofaceBuffLevel)
		return;

	for (int i = 0; i < nofaceBuffLevel; i++)
		currentStats -= nofaceBuff;
	
	for(int i = 0; i < _level; i++)
		currentStats += nofaceBuff;

	nofaceBuffLevel = _level;
}

CombatantAttributes& CombatantStatus::GetNofaceStats()
{
	CombatantAttributes attributes;
	attributes.Reset();

	for (int i = 0; i < nofaceBuffLevel; i++)
		attributes += nofaceBuff;

	return attributes;
}

int CombatantStatus::RoundsDamageOverTime()
{
	int maxRounds = 0;
	for (auto &d : damageOverTime)
	{
		if (d.first > maxRounds)
			maxRounds = d.first;
	}
	return maxRounds;
}


int CombatantStatus::DamageOverTime()
{
	int damage = 0;

	for (auto &d : damageOverTime)
		damage += d.second;

	return damage;
}

Buff & CombatantStatus::GetBuff()
{
	returnBuff.SetStandardValues();

	for (auto &b : buffs)
	{
		returnBuff.stats += b.stats;
		if(returnBuff.duration < b.duration)
			returnBuff.duration = b.duration;
	}

	return returnBuff;
}

Buff & CombatantStatus::GetDebuff()
{
	returnBuff.SetStandardValues();

	for (auto &b : debuffs)
	{
		returnBuff.stats += b.stats;
		if (returnBuff.duration < b.duration)
			returnBuff.duration = b.duration;
	}

	return returnBuff;
}


void CombatantStatus::RemoveAllBuffs()
{
	std::vector<Buff>::iterator i;
	for (i = buffs.begin(); i != buffs.end(); i++)
		currentStats -= i->stats;

	buffs.clear();

	notificationRenderer->AddNotification("Buffs entfernt!", g_pFonts->f_kingArthur, sf::Vector2f(combatant->GetRect().left - combatant->GetRect().width / 2.0f, combatant->GetRect().top - 20.0f), 1.0f);
}


void CombatantStatus::RemoveAllDebuffs()
{
	std::vector<Buff>::iterator i;
	for (i = debuffs.begin(); i != debuffs.end(); i++)
		currentStats -= i->stats;

	debuffs.clear();

	notificationRenderer->AddNotification("Debuffs entfernt!", g_pFonts->f_kingArthur, sf::Vector2f(combatant->GetRect().left - combatant->GetRect().width / 2.0f, combatant->GetRect().top - 20.0f), 1.0f);
}


int CombatantStatus::GetMaxHealth()
{
	return currentStats.maxHealth < 0 ? 0 : currentStats.maxHealth;
}


int CombatantStatus::GetCurrentHealth()
{
	return currentStats.currentHealth < 0 ? 0 : currentStats.currentHealth;
}


int CombatantStatus::GetDamage()
{
	return GetDamageMin() + rand() % (GetDamageMax() - GetDamageMin() + 1);
}

int CombatantStatus::GetDamageMin()
{
	return currentStats.damageMin < 0 ? 0 : currentStats.damageMin;
}

int CombatantStatus::GetDamageMax()
{
	return currentStats.damageMax < 0 ? 0 : currentStats.damageMax;
}

int CombatantStatus::GetArmour()
{
	return currentStats.armour < 0 ? 0 : currentStats.armour;
}

int CombatantStatus::GetCriticalHit()
{
	return currentStats.criticalHit < 0 ? 0 : currentStats.criticalHit;
}

int CombatantStatus::GetDodge()
{
	return currentStats.dodge < 0 ? 0 : currentStats.dodge;
}

int CombatantStatus::GetPrecision()
{
	return currentStats.precision < 0 ? 0 : currentStats.precision;
}


int CombatantStatus::GetInitiative()
{
	return currentStats.initiative < 0 ? 0 : currentStats.initiative;
}

void CombatantStatus::Reset()
{
	confused = 0;
	sleeping = false;
	marked = 0;
	damageOverTime.clear();
	buffs.clear();
	debuffs.clear();
	stupid = false;
	nofaceBuffLevel = -1;
}



