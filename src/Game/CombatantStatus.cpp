#include "CombatantStatus.hpp"



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
			attributes -= i->attributes;
			i = _buffs.erase(i);
		}
		else
			i++;
	}
}


void CombatantStatus::LooseHealth(int _damage)
{
	attributes.currentHealth -= _damage - ((float)attributes.armour / 100.0f * _damage);

	if (attributes.currentHealth < 0)
		attributes.currentHealth = 0;
}


void CombatantStatus::GainHealth(int _health)
{
	attributes.currentHealth += _health;

	if (attributes.currentHealth > attributes.maxHealth)
		attributes.currentHealth = attributes.maxHealth;
}


void CombatantStatus::AddBuff(Buff _buff)
{
	attributes += _buff.attributes;
	buffs.push_back(_buff);
}


void CombatantStatus::AddDebuff(Buff _buff)
{
	attributes += _buff.attributes;
	debuffs.push_back(_buff);
}


void CombatantStatus::RemoveAllBuffs()
{
	std::vector<Buff>::iterator i;
	for (i = buffs.begin(); i != buffs.end();)
		attributes -= i->attributes;

	buffs.clear();
}


void CombatantStatus::RemoveAllDebuffs()
{
	std::vector<Buff>::iterator i;
	for (i = debuffs.begin(); i != debuffs.end();)
		attributes -= i->attributes;

	debuffs.clear();
}


int CombatantStatus::GetMaxHealth()
{
	return attributes.maxHealth < 0 ? 0 : attributes.maxHealth;
}


int CombatantStatus::GetCurrentHealth()
{
	return attributes.currentHealth < 0 ? 0 : attributes.currentHealth;
}


int CombatantStatus::GetDamage()
{
	return attributes.damage < 0 ? 0 : attributes.damage;
}


int CombatantStatus::GetArmour()
{
	return attributes.armour < 0 ? 0 : attributes.armour;
}


int CombatantStatus::GetInitiative()
{
	return attributes.initiative < 0 ? 0 : attributes.initiative;
}

void CombatantStatus::Reset()
{
	confused = 0;
	sleeping = false;
	marked = 0;
	buffs.clear();
	debuffs.clear();
}



