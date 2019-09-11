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
	std::string tooltipString("");
	GenerateTooltipString(tooltipString, _ability);

	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(tooltipString);
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


void BattleGUI::GenerateTooltipString(std::string & _tooltip, int _ability)
{
	Ability &currentAbility = GetAbility(_ability);

	_tooltip.append("*#ffa500 " + GetAbility(_ability).name + "*\n");

	if (currentAbility.possibleAims.attackAll == true)
		_tooltip.append("#aa5000 Geht auf alle (Gegner und Freunde)\n");

	//fist of revenge special text
	if (_ability == 0 && combatantToDisplay->GetID() == CombatantID::Markus)
		_tooltip.append("#aa5000 Geht nur auf Gegner, die\nMarkus letze Runde angegriffen haben");

	if (currentAbility.precisionModificator != 0)
		_tooltip.append("#white Präzision: " + std::to_string(currentAbility.precisionModificator) + "\n");

	if (currentAbility.criticalHitModificator != 0)
		_tooltip.append("#white Kritische Trefferchance: " + std::to_string(currentAbility.criticalHitModificator) + "\n");

	if (currentAbility.canTargetEnemiesOrFriends)
	{
		_tooltip.append("#888888 Auf Freund\n");
		AppendTooltipStringForOneTarget(_tooltip, _ability, false, true);
		_tooltip.append("#888888 Auf Gegner\n");
		AppendTooltipStringForOneTarget(_tooltip, _ability, true, true);
	}
	else
		AppendTooltipStringForOneTarget(_tooltip, _ability, true, false);

	_tooltip.pop_back();
}


void BattleGUI::AppendTooltipStringForOneTarget(std::string & _tooltip, int _ability, bool _hostileAbility, bool _indent)
{
	Ability &currentAbility = GetAbility(_ability);
	AbilityEffect *effect;

	if (_hostileAbility)
		effect = &currentAbility.effectHostile;
	else
		effect = &currentAbility.effectFriendly;

	std::string indentation("");

	if (_indent)
		indentation = "\t";

	if (effect->damageFactor != 0)
		_tooltip.append(indentation + "#white Schadensfaktor: " + std::to_string(static_cast<int>(effect->damageFactor * 100)) + "%\n");

	if (currentAbility.lessTargetsMoreDamage != 0)
		_tooltip.append("#aa5000 Werden weniger als " + std::to_string(currentAbility.possibleAims.howMany) + " Gegner attackiert,\n"
			+ "steigt der Schadensfaktor pro fehlendem\nGegner um " + std::to_string(static_cast<int>(currentAbility.lessTargetsMoreDamage * 100)) + " Prozentpunkte\n");

	if (effect->confusion != 0)
		_tooltip.append(indentation + "#bb77bb Verwirrung (" + std::to_string(effect->confusion) + " Runden): " + std::to_string(static_cast<int>(effect->confusionProbability * 100)) + "%\n");

	if (effect->heal != 0)
		_tooltip.append(indentation + "#00aa00 Heilung: " + std::to_string(effect->heal) + "\n");

	if (effect->healSelf != 0)
		_tooltip.append(indentation + "#00aa00 Heilung (selbst): " + std::to_string(effect->healSelf) + "\n");

	if (effect->removeDebuffs != false)
		_tooltip.append(indentation + "#white Entfernt Debuffs\n");

	if (effect->removeBuffs != false)
		_tooltip.append(indentation + "#white Entfernt Buffs\n");

	if (effect->mark != 0)
		_tooltip.append(indentation + "#white Markiert (" + std::to_string(effect->mark) + " Runden)\n");

	if (effect->putToSleepProbability != 0)
		_tooltip.append(indentation + "#white Schlaf: " + std::to_string(static_cast<int>(effect->putToSleepProbability * 100)) + "%\n");

	if (effect->buff.duration != 0)
	{
		_tooltip.append(indentation + "#white Für " + std::to_string(effect->buff.duration) + " Runden:\n");

		std::string sign("");
		if (effect->buff.isPositive)
			sign = "+";

		if (effect->buff.stats.initiative != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.initiative) + " Initiative\n");

		if (effect->buff.stats.armour != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.armour) + " Rüstung\n");

		if (effect->buff.stats.damageMax != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.damageMax) + " Schaden\n");

		if (effect->buff.stats.criticalHit != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.criticalHit) + " Kritische Trefferchance\n");

		if (effect->buff.stats.dodge != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.dodge) + " Ausweichen\n");

		if (effect->buff.stats.precision != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.precision) + " Präzision\n");

		if (effect->buff.stats.maxHealth != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.maxHealth) + " Maximales Leben\n");

		if (effect->buff.stats.attributes.constitution != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.attributes.constitution) + " Konstitution\n");

		if (effect->buff.stats.attributes.dexterity != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.attributes.dexterity) + " Geschicklichkeit\n");

		if (effect->buff.stats.attributes.strength != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.attributes.strength) + " Stärke\n");

		if (effect->buff.stats.attributes.speed != 0)
			_tooltip.append(indentation + "\t#aaaadd " + sign + std::to_string(effect->buff.stats.attributes.speed) + " Geschwindigkeit\n");
	}
}

Ability &BattleGUI::GetAbility(int _ability)
{
	return g_pObjectProperties->playerAbilities[combatantToDisplay->GetID()][_ability];
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