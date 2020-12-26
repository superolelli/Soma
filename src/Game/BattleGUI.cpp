#include "BattleGUI.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "AdventureGroup.hpp"
#include "GameStatus.hpp"


void BattleGUI::Init(CGameEngine *_engine, GameStatus *_gameStatus)
{
	engine = _engine;

	commonGUIParts.Init(_engine, _gameStatus);

	abilityPanel.Load(g_pTextures->abilityPanel);
	abilityPanel.SetPos(130, 860);

	int x = 0;
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

	abilityInformationText.setCharacterSize(16);
	abilityInformationText.setStyle(sf::Text::Bold);
	abilityInformationText.setFont(g_pFonts->f_trajan);
	abilityInformationText.setFillColor(sf::Color(200, 200, 130));
	//abilityInformationText.setOutlineColor(sf::Color::Black);
	//abilityInformationText.setOutlineThickness(2.0);
	
	tooltip.Init();
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

	combatantResistancesPanel.Update(combatantToDisplay->Status());
	combatantResistancesPanel.SetPos(xPos - combatantResistancesPanel.GetRect().width - 30, yPos);

	commonGUIParts.Update();

	currentCombatantHealthBar.Update(g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds());

	abilityInformationText.setString("");

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

		if (!combatantToDisplay->IsPlayer() && dynamic_cast<Player*>(currentPlayer)->CurrentAbilityCanAimAtCombatant(combatantToDisplay))
			UpdateAbilityInformationText();
	}
}

void BattleGUI::SetCurrentAbility(int _ability)
{
	currentAbility = _ability;
	currentAbilityFrame.SetPos(abilities[currentPlayer->GetID()][_ability].GetRect().left - 10, abilities[currentPlayer->GetID()][_ability].GetRect().top - 10);
}


void BattleGUI::UpdateAbilityInformationText()
{
	float additionalDamageFactor = dynamic_cast<Player*>(currentPlayer)->GetAdditionalDamageForCurrentlyAimedCombatant();
	int hitProbability = 100 - (combatantToDisplay->Status().GetAttribute("dodge") - (currentPlayer->Status().GetAttribute("precision") + g_pObjectProperties->playerAbilities[currentPlayer->GetID()][currentAbility].precisionModificator)) * 2;
	hitProbability = std::max(0, std::min(100, hitProbability));
	int maxDamage = currentPlayer->Status().GetAttribute("damageMax") * (g_pObjectProperties->playerAbilities[currentPlayer->GetID()][currentAbility].effectHostile.damageFactor + additionalDamageFactor);
	maxDamage -= std::round(((float)combatantToDisplay->Status().GetAttribute("armour") / 100.0f * maxDamage));
	int minDamage = currentPlayer->Status().GetAttribute("damageMin") * (g_pObjectProperties->playerAbilities[currentPlayer->GetID()][currentAbility].effectHostile.damageFactor + additionalDamageFactor);
	minDamage -= std::round(((float)combatantToDisplay->Status().GetAttribute("armour") / 100.0f * minDamage));

	std::string abilityInformation = "";
	abilityInformation.append("Treffer: " + std::to_string(hitProbability) + "% | ");
	abilityInformation.append("Schaden: " + std::to_string(minDamage) + "-" + std::to_string(maxDamage));
	abilityInformationText.setString(abilityInformation);
	abilityInformationText.setPosition(currentCombatantHealthBar.GetRect().left + currentCombatantHealthBar.GetRect().width / 2 - abilityInformationText.getLocalBounds().width / 2, combatantAttributesPanel.GetRect().top + 145);

}


void BattleGUI::Render()
{
	if (currentPlayer != nullptr)
	{
		abilityPanel.Render(engine->GetRenderTarget());
		currentAbilityFrame.Render(engine->GetRenderTarget());

		for (CSprite &a : abilities[currentPlayer->GetID()])
			a.Render(engine->GetRenderTarget());

		for (int i = 0; i < 4; i++)
		{
			if (abilities[currentPlayer->GetID()][i].GetRect().contains(engine->GetMousePos()))
			{
				ShowTooltip(i);
			}
		}
	}

	commonGUIParts.Render();

	RenderCombatantInformation();	

	engine->GetRenderTarget().draw(abilityInformationText);
}


void BattleGUI::RenderCombatantInformation()
{
	combatantInformationPanel.Render(engine->GetRenderTarget());
	combatantAttributesPanel.Render(engine->GetRenderTarget());
	combatantResistancesPanel.Render(engine->GetRenderTarget());
	currentCombatantHealthBar.Render(engine->GetRenderTarget(), true);
	engine->GetRenderTarget().draw(currentCombatantName);
}


void BattleGUI::ShowTooltip(int _ability)
{
	tooltip.SetAbilityID(_ability);
	tooltip.ShowTooltip(engine->GetRenderTarget(), abilities[combatantToDisplay->GetID()][_ability].GetRect().left, abilities[combatantToDisplay->GetID()][_ability].GetRect().top - 35.0f);
	tooltip.ShowPossibleTargets(engine->GetRenderTarget(), abilityPanel.GetGlobalRect().left + 65, abilityPanel.GetGlobalRect().top + abilityPanel.GetGlobalRect().height + 5);
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


void BattleGUI::SetCurrentPlayer(Combatant *_combatant)
{
	currentPlayer = _combatant;

	if(currentPlayer)
		tooltip.SetPlayerID(currentPlayer->GetID());
}