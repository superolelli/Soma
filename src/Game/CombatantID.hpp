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
	Greg,
	Apachekid,
	BigSpencer,
	TequilaJoe,
	BillNoface,
	JuanTirador,
    number_of_combatants
};


static std::unordered_map<std::string, CombatantID> combatantIdentifierMap = {};