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
	abilityPanel.SetPos(130, 860);

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
	currentAbilityFrame.SetPos(abilities[0][0].GetRect().left - 10, abilities[0][0].GetRect().top - 10);

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
	
	tooltip.Init(engine);
	tooltip.SetShowAboveY(true);

	currentAbility = 0;
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
			{
				ShowTooltip(i);
			}
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
	tooltip.SetAbilityID(_ability);
	tooltip.ShowTooltip(abilities[combatantToDisplay->GetID()][_ability].GetRect().left, abilities[combatantToDisplay->GetID()][_ability].GetRect().top - 35.0f);
	tooltip.ShowPossibleTargets(abilityPanel.GetGlobalRect().left + 65, abilityPanel.GetGlobalRect().top + abilityPanel.GetGlobalRect().height + 5);
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

	if(currentPlayer)
		tooltip.SetPlayerID(currentPlayer->GetID());
}