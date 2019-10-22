#pragma once
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "Ability.hpp"
#include "../Framework/Gameengine.hpp"
#include "Tooltip.hpp"


class AbilityTooltip : public Tooltip
{
public:

	void Init(CGameEngine *_engine) override;

	void SetPlayerID(int _player);
	void SetAbilityID(int _ability);
	void ShowPossibleTargets(int _x, int _y, bool _showStacked = false);

private:
	int playerID;
	int abilityID;

	sf::Text targetsInformationText;
	sf::RectangleShape targetsBackground;

	CSprite possibleTargets[8];

	void GenerateTooltipString(std::string &_tooltip) override;
	void AppendTooltipStringForOneTarget(std::string & _tooltip, bool _hostileAbility, bool _indent);
	Ability &GetCurrentAbility();
};