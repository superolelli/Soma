#include "BattleGUI.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "AdventureGroup.hpp"
#include "Combatant.hpp"


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

	currentCombatantHealthBar.Load(g_pTextures->healthBarBig, g_pTextures->healthBarBigFrame, nullptr, nullptr);
	currentCombatantHealthBar.SetSmoothTransformationTime(0.7);
	currentCombatantHealthBar.SetOffsetForInnerPart(17, 20);
	currentCombatantHealthBar.SetPos(engine->GetWindowSize().x - 600.0f, 850.0f);
	currentCombatantHealthBar.SetText(g_pFonts->f_trajan, sf::Color::White, 14);

	currentCombatantName.setCharacterSize(25);
	currentCombatantName.setFont(g_pFonts->f_trajan);
	currentCombatantName.setFillColor(sf::Color::White);
	currentCombatantName.setOutlineColor(sf::Color::Black);
	currentCombatantName.setOutlineThickness(4.0);

	currentAbility = 3;
	currentCombatant = nullptr;
}



void BattleGUI::Update()
{
	currentCombatantHealthBar.Update(g_pTimer->GetElapsedTime().asSeconds());
	if (currentCombatant->IsPlayer())
	{
		if (engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
		{
			for (int i = 0; i < 4; i++)
			{
				if (abilities[currentCombatant->GetID()][i].GetRect().contains(engine->GetMousePos()))
				{
					currentAbility = i;
					currentAbilityFrame.SetPos(abilities[currentCombatant->GetID()][i].GetRect().left - 10, abilities[currentCombatant->GetID()][i].GetRect().top - 10);
				}
			}
		}
	}
}


void BattleGUI::Render()
{
	if (currentCombatant->IsPlayer())
	{
		for (CSprite &a : abilities[currentCombatant->GetID()])
			a.Render(engine->GetWindow());

		currentAbilityFrame.Render(engine->GetWindow());

		for (int i = 0; i < 4; i++)
		{
			if (abilities[currentCombatant->GetID()][i].GetRect().contains(engine->GetMousePos()))
				ShowTooltip(i);
		}
	}

	RenderCombatantInformation();	
}


void BattleGUI::RenderCombatantInformation()
{
	auto backgroundWidth = engine->GetWindowSize().x - currentCombatantName.getGlobalBounds().left;
	sf::RoundedRectangleShape background(sf::Vector2f(backgroundWidth, 225.0f), 8, 20);
	background.setFillColor(sf::Color(0, 0, 0, 150));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(sf::Vector2f(currentCombatantName.getGlobalBounds().left - 50, 855) + sf::Vector2f(-10.0f, -7.0f));

	engine->GetWindow().draw(background);
	ShowCombatantAttributes();
	currentCombatantHealthBar.Render(engine->GetWindow(), true);
	engine->GetWindow().draw(currentCombatantName);
}


void BattleGUI::ShowTooltip(int _ability)
{
	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(sf::String::fromUtf8(g_pObjectProperties->playerAbilities[currentCombatant->GetID()][_ability].description.begin(), g_pObjectProperties->playerAbilities[currentCombatant->GetID()][_ability].description.end()));
	tooltip.setPosition(abilities[currentCombatant->GetID()][_ability].GetRect().left, abilities[currentCombatant->GetID()][_ability].GetRect().top - tooltip.getLocalBounds().height - 25.0f);

	sf::FloatRect backgroundRect = tooltip.getLocalBounds();
	sf::RoundedRectangleShape background(sf::Vector2f(backgroundRect.width + 20.0f, backgroundRect.height + 20.0f), 8, 20);
	background.setFillColor(sf::Color(0, 0, 0, 220));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(tooltip.getPosition() + sf::Vector2f(-10.0f, -7.0f));

	engine->GetWindow().draw(background);
	engine->GetWindow().draw(tooltip);
}


void BattleGUI::ShowCombatantAttributes()
{
	auto status = currentCombatant->Status();
	std::stringstream attributeString1;
	std::stringstream attributeString2;
	std::stringstream valueString1;
	std::stringstream valueString2;

	attributeString1 << "Stärke:\n";
	attributeString1 << "Konstitution:\n";
	attributeString1 << "Geschicklichkeit:\n";
	attributeString1 << "Geschwindigkeit:";

	valueString1 << status.GetStrength() << std::endl;
	valueString1 << status.GetConstitution() << std::endl;
	valueString1 << status.GetDexterity() << std::endl;
	valueString1 << status.GetSpeed();

	attributeString2 << "Rüstung:\n";
	attributeString2 << "Schaden:\n";
	attributeString2 << "Initiative:\n";
	attributeString2 << "Kritische Trefferchance:\n";
	attributeString2 << "Ausweichen:\n";
	attributeString2 << "Präzision:";

	valueString2 << status.GetArmour() << std::endl;
	valueString2 << status.GetDamageMin() << "-" << status.GetDamageMax() << std::endl;
	valueString2 << status.GetInitiative() << std::endl;
	valueString2 << status.GetCriticalHit() << std::endl;
	valueString2 << status.GetDodge() << std::endl;
	valueString2 << status.GetPrecision();

	sf::Text attributeText1;
	attributeText1.setCharacterSize(16);
	attributeText1.setFont(g_pFonts->f_trajan);
	attributeText1.setFillColor(sf::Color(220, 220, 220));
	attributeText1.setOutlineColor(sf::Color::Black);
	attributeText1.setOutlineThickness(1.0f);
	attributeText1.setString(attributeString1.str());

	sf::Text attributeText2;
	attributeText2.setCharacterSize(16);
	attributeText2.setFont(g_pFonts->f_trajan);
	attributeText2.setFillColor(sf::Color(150, 150, 150));
	attributeText2.setOutlineColor(sf::Color::Black);
	attributeText2.setOutlineThickness(1.0f);
	attributeText2.setString(attributeString2.str());

	sf::Text valueText1;
	valueText1.setCharacterSize(16);
	valueText1.setFont(g_pFonts->f_trajan);
	valueText1.setFillColor(sf::Color(220, 220, 220));
	valueText1.setOutlineColor(sf::Color::Black);
	valueText1.setOutlineThickness(1.0f);
	valueText1.setString(valueString1.str());

	sf::Text valueText2;
	valueText2.setCharacterSize(16);
	valueText2.setFont(g_pFonts->f_trajan);
	valueText2.setFillColor(sf::Color(150, 150, 150));
	valueText2.setOutlineColor(sf::Color::Black);
	valueText2.setOutlineThickness(1.0f);
	valueText2.setString(valueString2.str());

	valueText2.setPosition(engine->GetWindowSize().x - 200.0f, 935.0f);
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


void BattleGUI::SetCurrentCombatant(Combatant *_combatant)
{
	currentCombatant = _combatant;
	currentCombatantHealthBar.SetMaxValuePtr(_combatant->Status().GetMaxHealthPointer());
	currentCombatantHealthBar.SetValuePtr(_combatant->Status().GetCurrentHealthPointer());
	currentCombatantName.setString(g_pStringContainer->combatantNames[_combatant->GetID()]); 
	currentCombatantName.setPosition(currentCombatantHealthBar.GetRect().left - currentCombatantName.getLocalBounds().width - 20, currentCombatantHealthBar.GetRect().top + currentCombatantName.getLocalBounds().height / 2 + 5);
}


void BattleGUI::SetAdventureGroup(AdventureGroup *_adventureGroup)
{
	players = _adventureGroup;
}