#pragma once

#include "../../Framework/Patterns/singleton.hpp"
#include <map>

#include <SFML\System.hpp>
#include <vector>


//the class for loading all properties at once
#define g_pStringContainer CStringContainer::Get()
class CStringContainer : public TSingleton<CStringContainer>
{
public:

	//loads all strings for the specific language
	void LoadStrings();

	std::vector<sf::String> speechBubbleContents[4];
};