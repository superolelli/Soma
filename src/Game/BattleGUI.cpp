#include "BattleGUI.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "AdventureGroup.hpp"


void BattleGUI::Init(CGameEngine *_engine)
{
	engine = _engine;
	int x = 0;

	for (int j = 0; j < 4; j++)
	{
		 x = 150;
		for (int i = 0; i < 4; i++)
		{
			abilities[j][i].Load(g_pTextures->abilities[j][i]);
			abilities[j][i].SetPos(x, 910);
			x += 120;
		}
	}

	currentAbilityFrame.Load(g_pTextures->currentAbilityFrame);
	currentAbilityFrame.SetPos(x - 130, 900);

	currentAbility = 3;
	currentPlayer = Simon;
}



void BattleGUI::Update()
{
	if (engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
	{
		for (int i = 0; i < 4; i++)
		{
			if (abilities[currentPlayer][i].GetRect().contains(engine->GetMousePos()))
			{
				currentAbility = i;
				currentAbilityFrame.SetPos(abilities[currentPlayer][i].GetRect().left - 10, abilities[currentPlayer][i].GetRect().top - 10);
			}
		}
	}
}


void BattleGUI::Render()
{
	for (CSprite a : abilities[currentPlayer])
		a.Render(engine->GetWindow());

	currentAbilityFrame.Render(engine->GetWindow());

	for (int i = 0; i < 4; i++)
	{
		if (abilities[currentPlayer][i].GetRect().contains(engine->GetMousePos()))
			ShowTooltip(i);
	}

	ShowPlayerAttributes();
}



void BattleGUI::ShowTooltip(int _ability)
{
	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(sf::String::fromUtf8(g_pObjectProperties->playerAbilities[currentPlayer][_ability].description.begin(), g_pObjectProperties->playerAbilities[currentPlayer][_ability].description.end()));
	tooltip.setPosition(abilities[currentPlayer][_ability].GetRect().left, abilities[currentPlayer][_ability].GetRect().top - tooltip.getLocalBounds().height - 25.0f);

	sf::FloatRect backgroundRect = tooltip.getLocalBounds();
	sf::RoundedRectangleShape background(sf::Vector2f(backgroundRect.width + 20.0f, backgroundRect.height + 20.0f), 8, 20);
	background.setFillColor(sf::Color(0, 0, 0, 220));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(tooltip.getPosition() + sf::Vector2f(-10.0f, -7.0f));

	engine->GetWindow().draw(background);
	engine->GetWindow().draw(tooltip);
}


void BattleGUI::ShowPlayerAttributes()
{
	auto playerStatus = players->GetPlayer(currentPlayer)->Status();
	std::stringstream attributeString1;
	std::stringstream attributeString2;
	std::stringstream valueString1;
	std::stringstream valueString2;

	attributeString1 << "Stärke:\n";
	attributeString1 << "Konstitution:\n";
	attributeString1 << "Geschicklichkeit:\n";
	attributeString1 << "Geschwindigkeit:";

	valueString1 << playerStatus.GetStrength() << std::endl;
	valueString1 << playerStatus.GetConstitution() << std::endl;
	valueString1 << playerStatus.GetDexterity() << std::endl;
	valueString1 << playerStatus.GetSpeed();

	attributeString2 << "Rüstung:\n";
	attributeString2 << "Schaden:\n";
	attributeString2 << "Initiative:\n";
	attributeString2 << "Kritische Trefferchance:\n";
	attributeString2 << "Ausweichen:\n";
	attributeString2 << "Präzision:";

	valueString2 << playerStatus.GetArmour() << std::endl;
	valueString2 << playerStatus.GetDamageMin() << "-" << playerStatus.GetDamageMax() << std::endl;
	valueString2 << playerStatus.GetInitiative() << std::endl;
	valueString2 << playerStatus.GetCriticalHit() << std::endl;
	valueString2 << playerStatus.GetDodge() << std::endl;
	valueString2 << playerStatus.GetPrecision();

	sf::Text attributeText1;
	attributeText1.setCharacterSize(18);
	attributeText1.setFont(g_pFonts->f_trajan);
	attributeText1.setFillColor(sf::Color(220, 220, 220));
	attributeText1.setOutlineColor(sf::Color::Black);
	attributeText1.setOutlineThickness(1.0f);
	attributeText1.setString(attributeString1.str());

	sf::Text attributeText2;
	attributeText2.setCharacterSize(18);
	attributeText2.setFont(g_pFonts->f_trajan);
	attributeText2.setFillColor(sf::Color(150, 150, 150));
	attributeText2.setOutlineColor(sf::Color::Black);
	attributeText2.setOutlineThickness(1.0f);
	attributeText2.setString(attributeString2.str());

	sf::Text valueText1;
	valueText1.setCharacterSize(18);
	valueText1.setFont(g_pFonts->f_trajan);
	valueText1.setFillColor(sf::Color(220, 220, 220));
	valueText1.setOutlineColor(sf::Color::Black);
	valueText1.setOutlineThickness(1.0f);
	valueText1.setString(valueString1.str());

	sf::Text valueText2;
	valueText2.setCharacterSize(18);
	valueText2.setFont(g_pFonts->f_trajan);
	valueText2.setFillColor(sf::Color(150, 150, 150));
	valueText2.setOutlineColor(sf::Color::Black);
	valueText2.setOutlineThickness(1.0f);
	valueText2.setString(valueString2.str());

	valueText2.setPosition(engine->GetWindowSize().x - 200.0f, abilities[currentPlayer][3].GetRect().top - 20.0f);
	attributeText2.setPosition(valueText2.getGlobalBounds().left - attributeText2.getLocalBounds().width - 10.0f, valueText2.getGlobalBounds().top - 2.0f);
	valueText1.setPosition(attributeText2.getGlobalBounds().left - 50.0f, valueText2.getGlobalBounds().top);
	attributeText1.setPosition(valueText1.getGlobalBounds().left - attributeText1.getLocalBounds().width - 10.0f, valueText2.getGlobalBounds().top - 2.0f);

	auto backgroundWidth = valueText2.getGlobalBounds().left + valueText2.getGlobalBounds().width - attributeText1.getGlobalBounds().left + 20.0f;

	sf::RoundedRectangleShape background(sf::Vector2f(backgroundWidth, attributeText2.getLocalBounds().height + 20.0f), 8, 20);
	background.setFillColor(sf::Color(0, 0, 0, 220));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(attributeText1.getPosition() + sf::Vector2f(-10.0f, -7.0f));

	engine->GetWindow().draw(background);
	engine->GetWindow().draw(attributeText2);
	engine->GetWindow().draw(attributeText1);
	engine->GetWindow().draw(valueText2);
	engine->GetWindow().draw(valueText1);
}


void BattleGUI::SetCurrentPlayer(int _player)
{
	if (_player >= 0 && _player < 4)
		currentPlayer = (PlayerID)_player;
}


void BattleGUI::SetAdventureGroup(AdventureGroup *_adventureGroup)
{
	players = _adventureGroup;
}