#include "BattleGUI.hpp"
#include "../engine/Graphics/RichText.hpp"
#include "../engine/Graphics/RoundedRectangleShape.hpp"
#include "AdventureGroup.hpp"
#include "GameStatus.hpp"


BattleGUI::BattleGUI(CGameEngine *_engine, LevelStatus *_levelStatus, NotificationRenderer *_notificationRenderer)
	: engine(_engine)
	, commonGUIParts(_engine, _levelStatus, _notificationRenderer)
	, abilityPanel(g_pTextures->abilityPanel)
	, currentAbilityFrame(g_pTextures->currentAbilityFrame)
	, combatantInformationPanel(g_pTextures->combatantInformationPanel)
	, skipTurnButton(g_pTextures->skipTurnButton, Buttontypes::Up)
	, currentCombatantHealthBar(g_pTextures->healthBarBig, g_pTextures->healthBarBigFrame, nullptr, nullptr)
	, currentAbility(0)
	, skipTurn(false)
	, combatantToDisplay(nullptr)
	, currentPlayer(nullptr)
{
	abilityPanel.SetPos(100, 860);

	int x = 0;
	for (int j = 0; j < 4; j++)
	{
		 x = abilityPanel.GetGlobalRect().position.x + 40;
		for (int i = 0; i < 4; i++)
		{
			abilities[j][i] = std::make_unique<CSprite>(g_pTextures->abilities[j][i]);
			abilities[j][i]->SetPos(x, abilityPanel.GetGlobalRect().position.y + 32);
			x += 132;
		}
	}

	currentAbilityFrame.SetPos(abilities[0][0]->GetRect().position.x - 10, abilities[0][0]->GetRect().position.y - 10);
	combatantInformationPanel.SetPos(820, 850);
	skipTurnButton.SetPos(abilityPanel.GetRect().position.x + abilityPanel.GetRect().size.x, abilityPanel.GetRect().position.y + abilityPanel.GetRect().size.y / 2 - skipTurnButton.GetRect().size.y / 2);

	currentCombatantHealthBar.SetSmoothTransformationTime(0.7);
	currentCombatantHealthBar.SetOffsetForInnerPart(17, 20);
	currentCombatantHealthBar.SetPos(combatantInformationPanel.GetGlobalRect().position.x + 40, combatantInformationPanel.GetGlobalRect().position.y + 70);
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
}



void BattleGUI::Update()
{
	int xPos = combatantInformationPanel.GetGlobalRect().position.x + combatantInformationPanel.GetRect().size.x - combatantAttributesPanel.GetRect().size.x - 30;
	int yPos = combatantInformationPanel.GetGlobalRect().position.y + (combatantInformationPanel.GetRect().size.y - combatantAttributesPanel.GetRect().size.y) / 2;
	combatantAttributesPanel.Update(combatantToDisplay->Status());
	combatantAttributesPanel.SetPos(xPos, yPos);

	combatantResistancesPanel.Update(combatantToDisplay->Status());
	combatantResistancesPanel.SetPos(xPos - combatantResistancesPanel.GetRect().size.x - 30, yPos);

	commonGUIParts.Update();

	currentCombatantHealthBar.Update(g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds());

	abilityInformationText.setString("");

	if (currentPlayer != nullptr)
	{
		if (engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
		{
			for (int i = 0; i < 4; i++)
			{
				if (abilities[currentPlayer->GetID()][i]->GetRect().contains(engine->GetMousePos()))
				{
					currentAbility = i;
					currentAbilityFrame.SetPos(abilities[currentPlayer->GetID()][i]->GetRect().position.x - 10, abilities[currentPlayer->GetID()][i]->GetRect().position.y - 10);
				}
			}
		}

		if (skipTurnButton.Update(*engine)) {
			skipTurn = true;
		}

		if (!combatantToDisplay->IsPlayer() && dynamic_cast<Player*>(currentPlayer)->CurrentAbilityCanAimAtCombatant(combatantToDisplay))
			UpdateAbilityInformationText();
	}
}

void BattleGUI::SetCurrentAbility(int _ability)
{
	currentAbility = _ability;
	currentAbilityFrame.SetPos(abilities[currentPlayer->GetID()][_ability]->GetRect().position.x - 10, abilities[currentPlayer->GetID()][_ability]->GetRect().position.y - 10);
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
	abilityInformationText.setPosition(sf::Vector2f(currentCombatantHealthBar.GetRect().position.x + currentCombatantHealthBar.GetRect().size.x / 2 - abilityInformationText.getLocalBounds().size.x / 2, combatantAttributesPanel.GetRect().position.y + 145));

}


void BattleGUI::Render()
{
	if (currentPlayer != nullptr)
	{
		abilityPanel.Render(engine->GetRenderTarget());
		currentAbilityFrame.Render(engine->GetRenderTarget());

		for (auto &a : abilities[currentPlayer->GetID()])
			a->Render(engine->GetRenderTarget());

		skipTurnButton.Render(engine->GetRenderTarget());

		for (int i = 0; i < 4; i++)
		{
			if (abilities[currentPlayer->GetID()][i]->GetRect().contains(engine->GetMousePos()))
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
	tooltip.ShowTooltip(engine->GetRenderTarget(), abilities[combatantToDisplay->GetID()][_ability]->GetRect().position.x, abilities[combatantToDisplay->GetID()][_ability]->GetRect().position.y - 35.0f);
	tooltip.ShowPossibleTargets(engine->GetRenderTarget(), abilityPanel.GetGlobalRect().position.x + 65, abilityPanel.GetGlobalRect().position.y + abilityPanel.GetGlobalRect().size.y + 5);
}


void BattleGUI::SetCombatantToDisplay(Combatant *_combatant)
{
	if (combatantToDisplay != _combatant)
	{
		combatantToDisplay = _combatant;
		currentCombatantHealthBar.SetMaxValuePtr(_combatant->Status().GetMaxHealthPointer());
		currentCombatantHealthBar.SetValuePtr(_combatant->Status().GetCurrentHealthPointer());
		currentCombatantName.setString(g_pObjectProperties->combatantNames[_combatant->GetID()]);

		int nameXPos = currentCombatantHealthBar.GetRect().position.x + (currentCombatantHealthBar.GetRect().size.x - currentCombatantName.getLocalBounds().size.x) / 2;
		currentCombatantName.setPosition(sf::Vector2f(nameXPos, currentCombatantHealthBar.GetRect().position.y - currentCombatantName.getLocalBounds().size.y - 10));
	}

}


void BattleGUI::SetCurrentPlayer(Combatant *_combatant)
{
	currentPlayer = _combatant;
	skipTurn = false;

	if(currentPlayer)
		tooltip.SetPlayerID(currentPlayer->GetID());
}