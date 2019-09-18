#pragma once
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "Ability.hpp"
#include "../Framework/Gameengine.hpp"


class AbilityTooltip
{
public:

	void Init(CGameEngine *_engine);

	void SetPlayerID(int _player);
	void SetAbilityID(int _ability);
	void SetShowAboveY(bool _aboveY);
	void ShowTooltip(int _x, int _y);
	void ShowPossibleTargets(int _x, int _y, bool _showStacked = false);

private:
	CGameEngine *engine;
	int playerID;
	int abilityID;

	bool showTooltipAboveY;

	sfe::RichText tooltipText;
	sf::Text targetsInformationText;
	sf::RoundedRectangleShape tooltipBackground;
	sf::RectangleShape targetsBackground;

	CSprite possibleTargets[8];

	void GenerateTooltipString(std::string &_tooltip);
	void AppendTooltipStringForOneTarget(std::string & _tooltip, bool _hostileAbility, bool _indent);
	Ability &GetCurrentAbility();
};