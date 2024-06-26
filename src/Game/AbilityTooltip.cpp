#include "AbilityTooltip.hpp"
#include "Resources\FontManager.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "Resources/Colors.hpp"

AbilityTooltip::AbilityTooltip()
	: Tooltip(false)
	, possibleTargets{{g_pTextures->possibleTargetSimon},
		{g_pTextures->possibleTargetOle},
		{g_pTextures->possibleTargetAnna},
		{g_pTextures->possibleTargetMarkus},
		{g_pTextures->possibleTargetEnemy},
		{g_pTextures->possibleTargetEnemy}, 
		{g_pTextures->possibleTargetEnemy}, 
		{g_pTextures->possibleTargetEnemy}}
	, playerID(0)
	, abilityID(0)
{
	targetsInformationText.setCharacterSize(16);
	targetsInformationText.setFont(g_pFonts->f_trajan);
	targetsInformationText.setFillColor(sf::Color::White);

	targetsBackground.setFillColor(sf::Color(0, 0, 0, 220));
	targetsBackground.setOutlineThickness(2.0f);
	targetsBackground.setOutlineColor(sf::Color(40, 40, 40));
}


void AbilityTooltip::SetPlayerID(int _player)
{
	playerID = _player;
}


void AbilityTooltip::SetAbilityID(int _ability)
{
	abilityID = _ability;
}


void AbilityTooltip::GenerateTooltipString(std::string & _tooltip)
{
	Ability &currentAbility = GetCurrentAbility();

	_tooltip.append("*" + Colors::NAME + currentAbility.name + "*\n");

	if (currentAbility.possibleAims.attackAll == true)
		_tooltip.append(Colors::INFO_HIGHLIGHT + "Trifft alle (Gegner und Freunde)\n");

	//fist of revenge special text
	if (abilityID == 0 && playerID == CombatantID::Markus)
		_tooltip.append(Colors::INFO_HIGHLIGHT + "Geht nur auf Gegner, die\nMarkus letze Runde angegriffen haben\n");

	if (currentAbility.fatigue != 0)
		_tooltip.append(Colors::FATIGUE + "M�digkeit: " + std::to_string(currentAbility.fatigue) + "\n");

	if (currentAbility.precisionModificator != 0)
		_tooltip.append(Colors::DEFAULT + "Pr�zision: " + std::to_string(currentAbility.precisionModificator) + "\n");

	if (currentAbility.canTargetEnemiesOrFriends)
	{
		if (currentAbility.possibleAims.attackAllPlayers)
			_tooltip.append(Colors::INFO + "Auf Heldengruppe\n");
		else
			_tooltip.append(Colors::INFO + "Auf Freund\n");

		AppendTooltipStringForOneTarget(_tooltip, false, true);
		_tooltip.append(Colors::INFO  + "Auf Gegner\n");
		AppendTooltipStringForOneTarget(_tooltip, true, true);
	}
	else
		AppendTooltipStringForOneTarget(_tooltip, true, false);

	_tooltip.pop_back();
}


void AbilityTooltip::AppendTooltipStringForOneTarget(std::string & _tooltip, bool _hostileAbility, bool _indent)
{
	Ability &currentAbility = GetCurrentAbility();
	AbilityEffect *effect;

	if (_hostileAbility)
		effect = &currentAbility.effectHostile;
	else
		effect = &currentAbility.effectFriendly;

	std::string indentation("");

	if (_indent)
		indentation = "\t";

	if (effect->criticalHitModificator != 0)
		_tooltip.append(Colors::DEFAULT + "Kritische Trefferchance: " + std::to_string(effect->criticalHitModificator) + "\n");

	if (effect->damageFactor != 0)
		_tooltip.append(indentation + Colors::DEFAULT + "Schadensfaktor: " + std::to_string(static_cast<int>(effect->damageFactor * 100)) + "%\n");

	if (effect->lessTargetsMoreDamage != 0)
		_tooltip.append(Colors::INFO_HIGHLIGHT + "Werden weniger als " + std::to_string(currentAbility.possibleAims.howMany) + " Gegner attackiert,\n"
			+ "steigt der Schadensfaktor pro fehlendem\nGegner um " + std::to_string(static_cast<int>(effect->lessTargetsMoreDamage * 100)) + " Prozentpunkte\n");

	if (effect->decayRounds != 0)
		_tooltip.append(indentation + Colors::DECAY  + "Verfall (" + DurationString(effect->decayRounds) + "): " + std::to_string(effect->decay) + " Schaden\n");

	if (effect->confusion != 0)
		_tooltip.append(indentation + Colors::CONFUSION + "Verwirrung (" + DurationString(effect->confusion) + "): " + std::to_string(static_cast<int>(effect->confusionProbability * 100)) + "%\n");

	if (effect->heal != 0)
		_tooltip.append(indentation + Colors::HEALING + "Heilung: " + std::to_string(effect->heal) + "\n");

	if (effect->healSelf != 0)
		_tooltip.append(indentation + Colors::HEALING + "Heilung (selbst): " + std::to_string(effect->healSelf) + "\n");

	if (effect->removeDebuffs != false)
		_tooltip.append(indentation + Colors::DEFAULT + "Entfernt Debuffs\n");

	if (effect->removeBuffs != false)
		_tooltip.append(indentation + Colors::DEFAULT + "Entfernt Buffs\n");

	if (effect->mark != 0)
		_tooltip.append(indentation + Colors::DEFAULT + "Markiert (" + DurationString(effect->mark) + ")\n");

	if (effect->putToSleepProbability != 0)
		_tooltip.append(indentation + Colors::DEFAULT + "Schlaf: " + std::to_string(static_cast<int>(effect->putToSleepProbability * 100)) + "%\n");

	if (effect->buff.duration != 0)
	{
		_tooltip.append(indentation + Colors::DEFAULT + "F�r " + DurationString(effect->buff.duration) + ":\n");

		std::string sign("");
		if (effect->buff.isPositive)
			sign = "+";

		AppendCombatantAttributesTooltip(_tooltip, effect->buff.stats, indentation + "\t", sign);
	}
}



Ability &AbilityTooltip::GetCurrentAbility()
{
	return g_pObjectProperties->playerAbilities[playerID][abilityID];
}



void AbilityTooltip::ShowPossibleTargets(sf::RenderTarget &_target, int _x, int _y, bool _showStacked)
{
	if (_showStacked)
		targetsBackground.setSize(sf::Vector2f(265, 70));
	else
		targetsBackground.setSize(sf::Vector2f(450, 40));

	targetsBackground.setPosition(_x, _y);
	
	int targetX;
	
	if (_showStacked)
		targetX = 70;
	else
		targetX = 315;

	for (int i = 0; i < 4; i++)
	{
		possibleTargets[i].SetPos(targetsBackground.getPosition().x + targetX, targetsBackground.getPosition().y - possibleTargets[i].GetGlobalRect().height + 37);
		targetX += possibleTargets[i].GetRect().width;
	}

	targetX += 10;

	for (int i = 4; i < 8; i++)
	{
		possibleTargets[i].SetPos(targetsBackground.getPosition().x + targetX, targetsBackground.getPosition().y - possibleTargets[i].GetGlobalRect().height + 37);
		targetX += possibleTargets[i].GetRect().width + 1;
	}

	int numberOfTargets = GetCurrentAbility().possibleAims.howMany;
	if (GetCurrentAbility().possibleAims.attackAll)
		targetsInformationText.setString("Die F�higkeit trifft ALLE.");
	else if (GetCurrentAbility().possibleAims.attackAllPlayers && GetCurrentAbility().possibleAims.howMany == 1)
		targetsInformationText.setString("Trifft 1 Ziel/alle Helden.");
	else if (numberOfTargets > 1)
		targetsInformationText.setString("Die F�higkeit trifft " + std::to_string(numberOfTargets) + " Ziele.");
	else
		targetsInformationText.setString("Die F�higkeit trifft 1 Ziel.");

	if(_showStacked)
		targetsInformationText.setPosition(targetsBackground.getPosition().x + 10, targetsBackground.getPosition().y + 43);
	else
		targetsInformationText.setPosition(targetsBackground.getPosition().x + 10, targetsBackground.getPosition().y + 10);

	_target.draw(targetsBackground);
	_target.draw(targetsInformationText);
	for (int i = 0; i < 8; i++)
	{
		if (GetCurrentAbility().possibleAims.position[i])
		{
			if (i < 4)
				possibleTargets[i].SetColor(0, 200, 0);
			else
				possibleTargets[i].SetColor(255, 0, 0);
		}
		else
			possibleTargets[i].SetColor(60, 60, 60);

		possibleTargets[i].Render(_target);
	}
}
