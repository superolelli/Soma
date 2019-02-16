#include "CombatantStatus.hpp"
#include "Resources\FontManager.hpp"
#include "Combatant.hpp"



void CombatantStatus::Init(Combatant *_combatant, NotificationRenderer *_notificationRenderer)
{
	combatant = _combatant;
	notificationRenderer = _notificationRenderer;
}


void CombatantStatus::HandleStatusChanges()
{
	if (marked > 0)
		marked--;
	if (confused > 0)
		confused--;
	if (sleeping = true)
		sleeping = false;

	HandleBuffDurations(buffs);
	HandleBuffDurations(debuffs);
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


void CombatantStatus::LooseHealth(int _damage, bool _critical)
{
	int damage = _damage - std::round(((float)currentStats.armour / 100.0f * _damage));
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
	for (i = buffs.begin(); i != buffs.end();)
		currentStats -= i->stats;

	buffs.clear();
}


void CombatantStatus::RemoveAllDebuffs()
{
	std::vector<Buff>::iterator i;
	for (i = debuffs.begin(); i != debuffs.end();)
		currentStats -= i->stats;

	debuffs.clear();
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
	buffs.clear();
	debuffs.clear();
}



