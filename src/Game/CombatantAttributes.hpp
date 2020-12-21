#pragma once

#include <unordered_map>


struct CombatantAttributes
{
	CombatantAttributes();

	std::unordered_map<std::string, int> attributes;
	static const std::unordered_map<std::string, std::string> attributeDisplayNames;

	void operator +=(const CombatantAttributes &_stats);
	friend CombatantAttributes operator+(CombatantAttributes _lhs, const CombatantAttributes& _rhs);
	void operator -=(const CombatantAttributes &_stats);
	int operator [] (const std::string& id) const;
	int& operator [](const std::string &id);

	void Reset();

private:
	static const std::vector<std::string> attributeIdentifiers;
};

