#include "StringManager.hpp"
#include "../../Framework/Animations/pugixml/pugixml.hpp"

void CStringContainer::LoadStrings()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/Strings.xml");

	for (xml_node& player : doc.child("Strings").child("SpeechBubbles").children())
	{
		int playerId = player.attribute("id").as_int();
		for (xml_node& speech : player.children())
		{
			std::string tempString = speech.text().as_string();
			speechBubbleContents[playerId].push_back(sf::String::fromUtf8(tempString.begin(), tempString.end()));
		}
	}
}