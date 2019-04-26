#include "StringManager.hpp"
#include "../../Framework/Animations/pugixml/pugixml.hpp"

void CStringContainer::LoadStrings()
{
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/Strings.xml");

	for (xml_node &name : doc.child("Strings").child("CombatantNames").children())
	{
		std::string tempString = name.text().as_string();
		combatantNames[name.attribute("id").as_int()] = sf::String::fromUtf8(tempString.begin(), tempString.end());
	}
}