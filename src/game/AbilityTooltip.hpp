#pragma once
#include "../engine/Graphics/RichText.hpp"
#include "../engine/Graphics/RoundedRectangleShape.hpp"
#include "../engine/Graphics/Sprite.hpp"
#include "Ability.hpp"
#include "../engine/Gameengine.hpp"
#include "Tooltip.hpp"


class AbilityTooltip : public Tooltip
{
public:

	AbilityTooltip();

	void SetPlayerID(int _player);
	void SetAbilityID(int _ability);
	void ShowPossibleTargets(sf::RenderTarget &_target, int _x, int _y, bool _showStacked = false);

private:
	int playerID;
	int abilityID;

	sf::Text targetsInformationText{SfmlCompat::defaultFont()};
	sf::RectangleShape targetsBackground;

	CSprite possibleTargets[8];

	void GenerateTooltipString(std::string &_tooltip) override;
	void AppendTooltipStringForOneTarget(std::string & _tooltip, bool _hostileAbility, bool _indent);
	Ability &GetCurrentAbility();
};