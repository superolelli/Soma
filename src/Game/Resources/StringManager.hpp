#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <SFML\System.hpp>


//the class for loading all properties at once
#define g_pStringContainer CStringContainer::Get()
class CStringContainer : public TSingleton<CStringContainer>
{
public:

	//loads all strings for the specific language
	void LoadStrings();

	sf::String combatantNames[10];
};