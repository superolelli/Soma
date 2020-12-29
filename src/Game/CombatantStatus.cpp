#include "CombatantStatus.hpp"
#include "Resources\FontManager.hpp"
#include "Combatant.hpp"



void CombatantStatus::Init(Combatant *_combatant, NotificationRenderer *_notificationRenderer)
{
	combatant = _combatant;
	notificationRenderer = _notificationRenderer;

	fatigueDebuff["damageMin"] = -3;
	fatigueDebuff["damageMax"] = -3;
	fatigueDebuff["dodge"] = -5;
	fatigueDebuff["precision"] = -5;
	fatigueDebuff["initiative"] = -1;

	nofaceBuff["damageMin"] = 2;
	nofaceBuff["damageMax"] = 2;
	nofaceBuff["criticalHit"] = 1;
	nofaceBuff["precision"] = 2;
	nofaceBuff["initiative"] = 1;
}


void CombatantStatus::UpdateStatusForNewTurn(double _initialWaitingTime)
{
	sleepChecked = false;
	decayChecked = false;
	dynamiteChecked = false;
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

	if (!dynamiteChecked)
	{
		HandleDynamite();
		dynamiteChecked = true;
	}
	else if (!decayChecked)
	{
		HandleDecay();
		decayChecked = true;
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
	return !sleepChecked || !decayChecked || !dynamiteChecked || statusAnnouncementTime > 0.0;
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

void CombatantStatus::HandleDecay()
{
	int damage = 0;
	std::vector<std::pair<int, int>>::iterator i;
	for (i = decay.begin(); i != decay.end();)
	{
		if (i->first > 0)
		{
			i->first--;
			damage += i->second;
		}

		if (i->first <= 0)
			i = decay.erase(i);
		else
			i++;
	}

	if (damage > 0)
	{
		LooseHealth(damage, false, false);
		statusAnnouncementTime = 2.0;
		g_pSounds->PlaySound(DAMAGE_OVER_TIME);

		if (GetAttribute("currentHealth") <= 0)
			skipRound = true;
	}
}

void CombatantStatus::HandleDynamite()
{
	if (dynamite)
	{
		if (rand() % 5 == 0)
		{
			LooseHealth(15, false);
			statusAnnouncementTime = 2.0;
			g_pSounds->PlaySound(DYNAMITE);

			if (GetAttribute("currentHealth") <= 0)
				skipRound = true;
		}
		else 
			combatant->CombatantAtNextPosition()->Status().AddDynamite();

		dynamite = false;
	}
}


void CombatantStatus::LooseHealth(int _damage, bool _critical, bool _useArmour)
{
	int damage = _damage;

	if(_useArmour)
		damage -= std::round(((float)currentStats["armour"] / 100.0f * _damage));

	currentStats["currentHealth"] -= damage;

	if (currentStats["currentHealth"] < 0)
		currentStats["currentHealth"] = 0;

	auto notificationPos = sf::Vector2f(combatant->GetRect().left + combatant->GetRect().width / 2.0f, combatant->GetRect().top);

	if(_critical)
		notificationRenderer->AddNotification(std::to_string(damage), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color(180, 0, 0), sf::Color::Black, 40);
	else
		notificationRenderer->AddNotification(std::to_string(damage), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color::Red, sf::Color::Black);

	CheckNofaceBuff();
}


void CombatantStatus::GainHealth(int _health, bool _critical)
{
	currentStats["currentHealth"] += _health;

	if (currentStats["currentHealth"] > currentStats["maxHealth"])
		currentStats["currentHealth"] = currentStats["maxHealth"];

	auto notificationPos = sf::Vector2f(combatant->GetRect().left + combatant->GetRect().width / 2.0f, combatant->GetRect().top);

	if(_critical)
		notificationRenderer->AddNotification(std::to_string(_health), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color(0, 180, 0), sf::Color::Black, 40);
	else
		notificationRenderer->AddNotification(std::to_string(_health), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color::Green, sf::Color::Black);

	CheckNofaceBuff();
}

void CombatantStatus::CheckNofaceBuff()
{
	if (nofaceBuffLevel >= 0)
	{
		SetNofaceBuffLevel((currentStats["maxHealth"] - currentStats["currentHealth"]) / 5);
	}
}


void CombatantStatus::AddDecay(int _rounds, int _damage)
{
	decay.push_back(std::pair<int, int>(_rounds, _damage));
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


void CombatantStatus::AddMiss(int _number)
{
	misses += _number;
	std::cout << misses << std::endl;
}

void CombatantStatus::RemoveMiss()
{
	misses -= 1;
	misses = std::max(0, misses);
}

int CombatantStatus::NumberOfMisses()
{
	return misses;
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

CombatantAttributes CombatantStatus::GetNofaceStats()
{
	CombatantAttributes attributes;
	attributes.Reset();

	for (int i = 0; i < nofaceBuffLevel; i++)
		attributes += nofaceBuff;

	return attributes;
}

int CombatantStatus::RoundsDecay()
{
	int maxRounds = 0;
	for (auto &d : decay)
	{
		if (d.first > maxRounds)
			maxRounds = d.first;
	}
	return maxRounds;
}


int CombatantStatus::GetDecay()
{
	int damage = 0;

	for (auto &d : decay)
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


int CombatantStatus::GetAttribute(const std::string& identifier)
{
	return currentStats[identifier] < 0 ? 0 : currentStats[identifier];
}


int CombatantStatus::GetDamage()
{
	return GetAttribute("damageMin") + rand() % (GetAttribute("damageMax") - GetAttribute("damageMin") + 1);
}

void CombatantStatus::Reset()
{
	confused = 0;
	sleeping = false;
	marked = 0;
	decay.clear();
	buffs.clear();
	debuffs.clear();
	stupid = false;
	nofaceBuffLevel = -1;
	misses = 0;
	dynamite = false;
}



