#include "CombatantResistancesPanel.hpp"
#include <sstream>
#include "Resources/SpritePool.hpp"


CombatantResistancesPanel::CombatantResistancesPanel()
{
	title.setCharacterSize(20);
	title.setFont(g_pFonts->f_trajan);
	title.setFillColor(sf::Color::White);
	title.setOutlineColor(sf::Color::Black);
	title.setOutlineThickness(2.0f);
	title.setString("Resistenzen");
	title.setPosition(0.0f, 0.0f);

	for (auto& text : resistancesText)
	{
		text.setCharacterSize(16);
		text.setFont(g_pFonts->f_trajan);
		text.setFillColor(sf::Color(220, 150, 0));
		text.setOutlineColor(sf::Color::Black);
		text.setOutlineThickness(2.0f);
	}
}

sf::FloatRect CombatantResistancesPanel::GetRect()
{
	sf::FloatRect rect = title.getGlobalBounds();

	for (auto &t : resistancesText)
	{
		if (t.getGlobalBounds().left + t.getGlobalBounds().width > rect.left + rect.width)
			rect.width = t.getGlobalBounds().left + t.getGlobalBounds().width - rect.left;
	}

	return rect;
}

void CombatantResistancesPanel::SetPos(float _x, float _y)
{
	title.setPosition(_x, _y);

	int yPos = title.getGlobalBounds().top + 30;
	for (auto& text : resistancesText)
	{
		text.setPosition(_x + 70, yPos);
		yPos += 25;
	}
}

void CombatantResistancesPanel::Update(CombatantStatus& _status)
{
	Buff buff = _status.GetBuff();
	Buff debuff = _status.GetDebuff();

	std::vector<std::string> toShow = { "sleepResistance", "confusionResistance", "debuffResistance", "decayResistance"};
	for (int i = 0; i < 4; i++)
	{
		resistancesText[i].setString(std::to_string(_status.GetAttribute(toShow[i])) + "%");
		SetValueTextColor(resistancesText[i], buff.stats[toShow[i]], debuff.stats[toShow[i]]);
	}

}


void CombatantResistancesPanel::SetValueTextColor(sf::Text& text, int buffValue, int debuffValue)
{
	int resultValue = buffValue + debuffValue;
	if (resultValue > 0)
		text.setFillColor(sf::Color::Green);
	else if (resultValue < 0)
		text.setFillColor(sf::Color::Red);
	else
	{
		text.setFillColor(sf::Color(220, 150, 0));
	}
}


void CombatantResistancesPanel::Render(sf::RenderTarget& _target)
{
	_target.draw(title);

	for (auto& text : resistancesText)
		_target.draw(text);

	g_pSpritePool->sleeping->SetPos(title.getGlobalBounds().left + 40, resistancesText[0].getGlobalBounds().top - 2);
	g_pSpritePool->sleeping->Render(_target);

	g_pSpritePool->confused->SetPos(title.getGlobalBounds().left + 40, resistancesText[1].getGlobalBounds().top - 2);
	g_pSpritePool->confused->Render(_target);

	g_pSpritePool->debuff->SetPos(title.getGlobalBounds().left + 40, resistancesText[2].getGlobalBounds().top - 2);
	g_pSpritePool->debuff->Render(_target);

	g_pSpritePool->decay->SetPos(title.getGlobalBounds().left + 40, resistancesText[3].getGlobalBounds().top - 2);
	g_pSpritePool->decay->Render(_target);
}