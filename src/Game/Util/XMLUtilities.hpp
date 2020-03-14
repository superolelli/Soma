#pragma once

#include "../../Framework/Animations/pugixml/pugixml.hpp"

#include "../Ability.hpp"
#include "../LevelSpecs.hpp"
#include "../Item.hpp"


namespace pugi {

    void loadBuffFromXML(const xml_node& buffNode, Buff &buff);

    void loadAbilityEffectFromXML(const xml_node& effectNode, AbilityEffect &effect);

	void loadAbilityFromXML(const xml_node& abilityNode, Ability &ability);

	void loadAttributesFromXML(const xml_node& attributeNode, CombatantAttributes &stats);

	void loadLevelSpecsFromXML(const xml_node& levelSpecNode, LevelSpecs &specs);

	void loadEnemyGroupFromXML(const xml_node& enemyGroupNode, std::array<CombatantID, 4> &newGroup);

	void loadPossibleLootFromXML(const xml_node& enemyNode, std::vector<ItemID> &loot);
}