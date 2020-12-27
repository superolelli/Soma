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
	background.setPosition(statsText.getPosition() + sf::Vector2f(-10.0f, -7.0f));
}

void CombatantAttributesPanel::InitDescriptionTexts()
{
	std::stringstream statsString;

	statsString << "Rüstung:\n";
	statsString << "Schaden:\n";
	statsString << "Initiative:\n";
	statsString << "Kritische Trefferchance:\n";
	statsString << "Ausweichen:\n";
	statsString << "Präzision:";

	statsText.setCharacterSize(16);
	statsText.setFont(g_pFonts->f_trajan);
	statsText.setFillColor(sf::Color(150, 150, 150));
	statsText.setOutlineColor(sf::Color::Black);
	statsText.setOutlineThickness(1.0f);
	statsText.setString(statsString.str());
	statsText.setPosition(0.0f, 0.0f);
}

void CombatantAttributesPanel::InitValueTexts()
{
	int yPos = statsText.getGlobalBounds().top;
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
	statsText.setPosition(_x + 10.0f, _y + 7.0f);

	int yPos = statsText.getGlobalBounds().top;
	for (auto &text : statValueText)
	{
		text.setPosition(statsText.getGlobalBounds().left + statsText.getGlobalBounds().width + 10.0f, yPos);
		yPos += 20;
	}

	background.setPosition(statsText.getPosition() + sf::Vector2f(-10.0f, -7.0f));
}

void CombatantAttributesPanel::Update(CombatantStatus &_status)
{
	Buff buff = _status.GetBuff();
	Buff debuff = _status.GetDebuff();

	std::vector<std::string> toShow = { "armour", "damageMin", "initiative", "criticalHit", "dodge", "precision" };
	for (int i = 0; i < 6 ; i++)
	{
		if(toShow[i] == "damageMin")
			statValueText[i].setString(std::to_string(_status.GetAttribute("damageMin")) + "-" + std::to_string(_status.GetAttribute("damageMax")));	
		else
			statValueText[i].setString(std::to_string(_status.GetAttribute(toShow[i])));

		SetValueTextColor(statValueText[i], buff.stats[toShow[i]], debuff.stats[toShow[i]]);
	}

	auto backgroundWidth = statValueText[0].getGlobalBounds().left - statsText.getGlobalBounds().left + 50.0f;
	background.setSize(sf::Vector2f(backgroundWidth, statsText.getLocalBounds().height + 20.0f));	

}


void CombatantAttributesPanel::SetValueTextColor(sf::Text &text, int buffValue, int debuffValue)
{
	int resultValue = buffValue + debuffValue;
	if (resultValue > 0)
		text.setFillColor(sf::Color::Green);
	else if (resultValue < 0)
		text.setFillColor(sf::Color::Red);
	else
	{
		text.setFillColor(sf::Color(220, 220, 220));
	}
}


void CombatantAttributesPanel::Render(sf::RenderTarget &_target)
{
	_target.draw(background);
	_target.draw(statsText);

	for (auto &text : statValueText)
		_target.draw(text);
}