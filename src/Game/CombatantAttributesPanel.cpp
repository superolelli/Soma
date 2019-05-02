#include "CombatantAttributesPanel.hpp"
#include <sstream>



CombatantAttributesPanel::CombatantAttributesPanel()
{
	InitDescriptionTexts();
	InitValueTexts();

	background.setCornerPointCount(20);
	background.setCornersRadius(8);
	background.setFillColor(sf::Color(0, 0, 0, 220));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(attributeText.getPosition() + sf::Vector2f(-10.0f, -7.0f));
}

void CombatantAttributesPanel::InitDescriptionTexts()
{
	std::stringstream attributeString;
	std::stringstream statsString;

	attributeString << "Stärke:\n";
	attributeString << "Konstitution:\n";
	attributeString << "Geschicklichkeit:\n";
	attributeString << "Geschwindigkeit:";

	statsString << "Rüstung:\n";
	statsString << "Schaden:\n";
	statsString << "Initiative:\n";
	statsString << "Kritische Trefferchance:\n";
	statsString << "Ausweichen:\n";
	statsString << "Präzision:";

	attributeText.setCharacterSize(16);
	attributeText.setFont(g_pFonts->f_trajan);
	attributeText.setFillColor(sf::Color(220, 220, 220));
	attributeText.setOutlineColor(sf::Color::Black);
	attributeText.setOutlineThickness(1.0f);
	attributeText.setString(attributeString.str());
	attributeText.setPosition(0.0f, 0.0f);

	statsText.setCharacterSize(16);
	statsText.setFont(g_pFonts->f_trajan);
	statsText.setFillColor(sf::Color(150, 150, 150));
	statsText.setOutlineColor(sf::Color::Black);
	statsText.setOutlineThickness(1.0f);
	statsText.setString(statsString.str());
	statsText.setPosition(attributeText.getLocalBounds().width + 60.0f, 0.0f);
}

void CombatantAttributesPanel::InitValueTexts()
{
	int yPos = attributeText.getGlobalBounds().top;
	for (auto &text : attributeValueText)
	{
		text.setCharacterSize(16);
		text.setFont(g_pFonts->f_trajan);
		text.setFillColor(sf::Color(220, 220, 220));
		text.setOutlineColor(sf::Color::Black);
		text.setOutlineThickness(1.0f);
		text.setPosition(attributeText.getGlobalBounds().left + attributeText.getGlobalBounds().width + 10.0f, yPos);
		yPos += 20;
	}

	yPos = statsText.getGlobalBounds().top;
	for (auto &text : statValueText)
	{
		text.setCharacterSize(16);
		text.setFont(g_pFonts->f_trajan);
		text.setFillColor(sf::Color(220, 220, 220));
		text.setOutlineColor(sf::Color::Black);
		text.setOutlineThickness(1.0f);
		text.setPosition(statsText.getGlobalBounds().left + statsText.getGlobalBounds().width + 10.0f, yPos);
		yPos += 20;
	}
}


void CombatantAttributesPanel::SetPos(float _x, float _y)
{
	attributeText.setPosition(_x + 10.0f, _y + 7.0f);
	statsText.setPosition(_x + attributeText.getLocalBounds().width + 70.0f, _y + 7.0f);

	int yPos = attributeText.getGlobalBounds().top;
	for (auto &text : attributeValueText)
	{
		text.setPosition(attributeText.getGlobalBounds().left + attributeText.getGlobalBounds().width + 10.0f, yPos);
		yPos += 20;
	}

	yPos = statsText.getGlobalBounds().top;
	for (auto &text : statValueText)
	{
		text.setPosition(statsText.getGlobalBounds().left + statsText.getGlobalBounds().width + 10.0f, yPos);
		yPos += 20;
	}

	background.setPosition(attributeText.getPosition() + sf::Vector2f(-10.0f, -7.0f));
}

void CombatantAttributesPanel::Update(CombatantStatus &_status)
{
	Buff buff = _status.GetBuff();
	Buff debuff = _status.GetDebuff();

	attributeValueText[0].setString(std::to_string(_status.GetStrength()));
	SetValueTextColor(attributeValueText[0], buff.stats.attributes.strength, debuff.stats.attributes.strength, true);

	attributeValueText[1].setString(std::to_string(_status.GetConstitution()));
	SetValueTextColor(attributeValueText[1], buff.stats.attributes.constitution, debuff.stats.attributes.constitution, true);

	attributeValueText[2].setString(std::to_string(_status.GetDexterity()));
	SetValueTextColor(attributeValueText[2], buff.stats.attributes.dexterity, debuff.stats.attributes.dexterity, true);

	attributeValueText[3].setString(std::to_string(_status.GetSpeed()));
	SetValueTextColor(attributeValueText[3], buff.stats.attributes.speed, debuff.stats.attributes.speed, true);


	statValueText[0].setString(std::to_string(_status.GetArmour()));
	SetValueTextColor(statValueText[0], buff.stats.armour, debuff.stats.armour, false);

	statValueText[1].setString(std::to_string(_status.GetDamageMin()) + "-" + std::to_string(_status.GetDamageMax()));
	SetValueTextColor(statValueText[1], buff.stats.damageMin, debuff.stats.damageMin, false);

	statValueText[2].setString(std::to_string(_status.GetInitiative()));
	SetValueTextColor(statValueText[2], buff.stats.initiative, debuff.stats.initiative, false);

	statValueText[3].setString(std::to_string(_status.GetCriticalHit()));
	SetValueTextColor(statValueText[3], buff.stats.criticalHit, debuff.stats.criticalHit, false);

	statValueText[4].setString(std::to_string(_status.GetDodge()));
	SetValueTextColor(statValueText[4], buff.stats.dodge, debuff.stats.dodge, false);

	statValueText[5].setString(std::to_string(_status.GetPrecision()));
	SetValueTextColor(statValueText[5], buff.stats.precision, debuff.stats.precision, false);

	auto backgroundWidth = statValueText[0].getGlobalBounds().left - attributeText.getGlobalBounds().left + 50.0f;
	background.setSize(sf::Vector2f(backgroundWidth, statsText.getLocalBounds().height + 20.0f));	

}


void CombatantAttributesPanel::SetValueTextColor(sf::Text &text, int buffValue, int debuffValue, bool _attribute)
{
	int resultValue = buffValue + debuffValue;
	if (resultValue > 0)
		text.setFillColor(sf::Color::Green);
	else if (resultValue < 0)
		text.setFillColor(sf::Color::Red);
	else
	{
		if (_attribute)
			text.setFillColor(sf::Color(220, 220, 220));
		else
			text.setFillColor(sf::Color(150, 150, 150));
	}
}


void CombatantAttributesPanel::Render(sf::RenderTarget &_target)
{
	_target.draw(background);
	_target.draw(attributeText);
	_target.draw(statsText);

	for(auto &text : attributeValueText)
		_target.draw(text);

	for (auto &text : statValueText)
		_target.draw(text);
}