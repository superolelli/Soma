#pragma once

#include <unordered_map>

enum CombatantID { 
	Undefined = -1, 
	Simon, 
	Ole, 
	Anna, 
	Markus, 
	Gesetzloser, 
	Abtruenniger, 
	Indianer, 
	Hilfssheriff,
	Greg
};


static std::unordered_map<std::string, CombatantID> combatantIdentifierMap = {
	{ "Simon", CombatantID::Simon },
	{ "Ole", CombatantID::Ole },
	{ "Anna", CombatantID::Anna },
	{ "Markus", CombatantID::Markus },
	{ "Gesetzloser", CombatantID::Gesetzloser },
	{ "Abtrünniger", CombatantID::Abtruenniger },
	{ "Indianer", CombatantID::Indianer },
	{ "Hilfssheriff", CombatantID::Hilfssheriff },
	{ "Greg Digger", CombatantID::Greg}

};