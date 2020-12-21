#include "CombatantAttributes.hpp"


const std::vector<std::string> CombatantAttributes::attributeIdentifiers = {
	"maxHealth", 
	"currentHealth", 
	"damageMin",
	"damageMax",
	"armour",
	"criticalHit",
	"dodge",
	"initiative",
	"precision"
};

const std::unordered_map<std::string, std::string> CombatantAttributes::attributeDisplayNames = {
	{"maxHealth",     "Maximales Leben"},
	{"currentHealth", "Derzeitiges Leben"},
	{"damageMin",	  "Schaden"},
	{"damageMax",	  "Schaden"},
	{"armour",		  "Rüstung"},
	{"criticalHit",	  "Kritische Trefferchance"},
	{"dodge",		  "Ausweichen"},
	{"initiative",	  "Initiative"},
	{"precision",	  "Präzision"}
};					  


CombatantAttributes::CombatantAttributes()
{
	Reset();
}


void CombatantAttributes::operator+=(const CombatantAttributes & _stats)
{
	for (auto& attr : attributes)
		attr.second += _stats.attributes.at(attr.first);
}


void CombatantAttributes::operator-=(const CombatantAttributes & _stats)
{
	for (auto& attr : attributes)
		attr.second -= _stats.attributes.at(attr.first);
}


void CombatantAttributes::Reset()
{
	for (auto& id : attributeIdentifiers)
		attributes[id] = 0;
}


int& CombatantAttributes::operator [](const std::string& id)
{
	return attributes[id];
}

int CombatantAttributes::operator [] (const std::string& id) const
{
	return attributes.at(id);
}

CombatantAttributes operator+(CombatantAttributes _lhs, const CombatantAttributes & _rhs)
{
	_lhs += _rhs;
	return _lhs;
}
