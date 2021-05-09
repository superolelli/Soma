#include "PlayerAttributesText.hpp"
#include <sstream>

PlayerAttributesText::PlayerAttributesText()
{
	InitDescriptionTexts();
	InitValueTexts();
}

void PlayerAttributesText::InitDescriptionTexts()
{
	std::stringstream statsString;

	statsString << "Rüstung:\n";
	statsString << "Schaden:\n";
	statsString << "Initiative:\n";
	statsString << "Krit. Chance:\n";
	statsString << "Ausweichen:\n";
	statsString << "Präzision:\n";
	statsString << "Heilungsmod.:\n";
	statsString << "Schlaf-Res.:\n";
	statsString << "Verwirrungs-Res.:\n";
	statsString << "Debuff-Res.:\n";
	statsString << "Verfall-Res.:";

	statsText.setCharacterSize(16);
	statsText.setFont(g_pFonts->f_trajan);
	statsText.setFillColor(sf::Color::White);
	statsText.setOutlineColor(sf::Color::Black);
	statsText.setOutlineThickness(1.0f);
	statsText.setString(statsString.str());
	statsText.setPosition(0.0f, 0.0f);
	statsText.setLineSpacing(1.7f);
}

void PlayerAttributesText::InitValueTexts()
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

void PlayerAttributesText::Render(sf::RenderTarget & _target)
{
	_target.draw(statsText);

	for (auto &text : statValueText)
		_target.draw(text);
}

void PlayerAttributesText::Update(CombatantAttributes const & _status)
{
	statValueText[0].setString(std::to_string(_status["armour"]));
	statValueText[1].setString(std::to_string(_status["damageMin"]) + "-" + std::to_string(_status["damageMax"]));
	statValueText[2].setString(std::to_string(_status["initiative"]));
	statValueText[3].setString(std::to_string(_status["criticalHit"]) + "%");
	statValueText[4].setString(std::to_string(_status["dodge"]));
	statValueText[5].setString(std::to_string(_status["precision"]));
	statValueText[6].setString(std::to_string(_status["healing"]) + "%");
	statValueText[7].setString(std::to_string(_status["sleepResistance"]) + "%");
	statValueText[8].setString(std::to_string(_status["confusionResistance"]) + "%");
	statValueText[9].setString(std::to_string(_status["debuffResistance"]) + "%");
	statValueText[10].setString(std::to_string(_status["decayResistance"]) + "%");
}

void PlayerAttributesText::SetPos(float _x, float _y)
{
	statsText.setPosition(_x, _y);

	float yPos = statsText.getGlobalBounds().top;
	for (auto &text : statValueText)
	{
		text.setPosition(statsText.getGlobalBounds().left + statsText.getGlobalBounds().width + 10.0f, yPos);
		yPos += 32.4f;
	}
}
