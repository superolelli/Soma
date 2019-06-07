#include "BattleGUI.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "AdventureGroup.hpp"
#include "Combatant.hpp"


void BattleGUI::Init(CGameEngine *_engine)
{
	engine = _engine;
	int x = 0;

	abilityPanel.Load(g_pTextures->abilityPanel);
	abilityPanel.SetPos(130, 870);

	for (int j = 0; j < 4; j++)
	{
		 x = abilityPanel.GetGlobalRect().left + 40;
		for (int i = 0; i < 4; i++)
		{
			abilities[j][i].Load(g_pTextures->abilities[j][i]);
			abilities[j][i].SetPos(x, abilityPanel.GetGlobalRect().top + 32);
			x += 132;
		}
	}

	currentAbilityFrame.Load(g_pTextures->currentAbilityFrame);
	currentAbilityFrame.SetPos(abilities[0][3].GetRect().left - 10, abilities[0][3].GetRect().top - 10);

	combatantInformationPanel.Load(g_pTextures->combatantInformationPanel);
	combatantInformationPanel.SetPos(820, 850);

	currentCombatantHealthBar.Load(g_pTextures->healthBarBig, g_pTextures->healthBarBigFrame, nullptr, nullptr);
	currentCombatantHealthBar.SetSmoothTransformationTime(0.7);
	currentCombatantHealthBar.SetOffsetForInnerPart(17, 20);
	currentCombatantHealthBar.SetPos(combatantInformationPanel.GetGlobalRect().left + 40, combatantInformationPanel.GetGlobalRect().top + 70);
	currentCombatantHealthBar.SetText(g_pFonts->f_trajan, sf::Color::White, 14);

	currentCombatantName.setCharacterSize(25);
	currentCombatantName.setFont(g_pFonts->f_trajan);
	currentCombatantName.setFillColor(sf::Color::White);
	currentCombatantName.setOutlineColor(sf::Color::Black);
	currentCombatantName.setOutlineThickness(4.0);

	currentAbility = 3;
	combatantToDisplay = nullptr;
	currentPlayer = nullptr;
}



void BattleGUI::Update()
{

	int xPos = combatantInformationPanel.GetGlobalRect().left + combatantInformationPanel.GetRect().width - combatantAttributesPanel.GetRect().width - 30;
	int yPos = combatantInformationPanel.GetGlobalRect().top + (combatantInformationPanel.GetRect().height - combatantAttributesPanel.GetRect().height) / 2;
	combatantAttributesPanel.Update(combatantToDisplay->Status());
	combatantAttributesPanel.SetPos(xPos, yPos);

	currentCombatantHealthBar.Update(g_pTimer->GetElapsedTime().asSeconds());

	if (currentPlayer != nullptr)
	{
		if (engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
		{
			for (int i = 0; i < 4; i++)
			{
				if (abilities[currentPlayer->GetID()][i].GetRect().contains(engine->GetMousePos()))
				{
					currentAbility = i;
					currentAbilityFrame.SetPos(abilities[currentPlayer->GetID()][i].GetRect().left - 10, abilities[currentPlayer->GetID()][i].GetRect().top - 10);
				}
			}
		}
	}
}


void BattleGUI::Render()
{
	if (currentPlayer != nullptr)
	{
		abilityPanel.Render(engine->GetWindow());
		currentAbilityFrame.Render(engine->GetWindow());

		for (CSprite &a : abilities[currentPlayer->GetID()])
			a.Render(engine->GetWindow());

		for (int i = 0; i < 4; i++)
		{
			if (abilities[currentPlayer->GetID()][i].GetRect().contains(engine->GetMousePos()))
				ShowTooltip(i);
		}
	}

	RenderCombatantInformation();	
}


void BattleGUI::RenderCombatantInformation()
{
	combatantInformationPanel.Render(engine->GetWindow());
	combatantAttributesPanel.Render(engine->GetWindow());
	currentCombatantHealthBar.Render(engine->GetWindow(), true);
	engine->GetWindow().draw(currentCombatantName);
}


void BattleGUI::ShowTooltip(int _ability)
{
	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(g_pObjectProperties->playerAbilities[combatantToDisplay->GetID()][_ability].description);
	tooltip.setPosition(abilities[combatantToDisplay->GetID()][_ability].GetRect().left, abilities[combatantToDisplay->GetID()][_ability].GetRect().top - tooltip.getLocalBounds().height - 25.0f);

	sf::FloatRect backgroundRect = tooltip.getLocalBounds();
	sf::RoundedRectangleShape background(sf::Vector2f(backgroundRect.width + 20.0f, backgroundRect.height + 20.0f), 8, 20);
	background.setFillColor(sf::Color(0, 0, 0, 220));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(tooltip.getPosition() + sf::Vector2f(-10.0f, -7.0f));

	engine->GetWindow().draw(background);
	engine->GetWindow().draw(tooltip);
}


void BattleGUI::SetCombatantToDisplay(Combatant *_combatant)
{
	if (combatantToDisplay != _combatant)
	{
		combatantToDisplay = _combatant;
		currentCombatantHealthBar.SetMaxValuePtr(_combatant->Status().GetMaxHealthPointer());
		currentCombatantHealthBar.SetValuePtr(_combatant->Status().GetCurrentHealthPointer());
		currentCombatantName.setString(g_pStringContainer->combatantNames[_combatant->GetID()]);

		int nameXPos = currentCombatantHealthBar.GetRect().left + (currentCombatantHealthBar.GetRect().width - currentCombatantName.getLocalBounds().width) / 2;
		currentCombatantName.setPosition(nameXPos, currentCombatantHealthBar.GetRect().top - currentCombatantName.getLocalBounds().height - 10);
	}

}

void BattleGUI::SetAdventureGroup(AdventureGroup *_adventureGroup)
{
	players = _adventureGroup;
}


void BattleGUI::SetCurrentPlayer(Combatant *_combatant)
{
	currentPlayer = _combatant;
}