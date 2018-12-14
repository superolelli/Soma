#pragma once

#include "../../Framework/Animations/pugixml/pugixml.hpp"

#include "../Ability.hpp"


namespace pugi {

    void loadBuffFromXML(const xml_node& buffNode, Buff &buff);

    void loadAbilityEffectFromXML(const xml_node& effectNode, AbilityEffect &effect);

	void loadAbilityFromXML(const xml_node& abilityNode, PlayerAbility &ability);

}