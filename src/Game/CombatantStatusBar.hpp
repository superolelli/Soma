#pragma once

#include "../Framework/Gui/Bar.hpp"
#include "CombatantStatus.hpp"
#include "CombatantStatusTooltip.hpp"

class CombatantStatusBar
{
public:

	CombatantStatusBar(CombatantStatus *_status, CGameEngine *_engine);
	void Render();
	void Update(sf::IntRect &_combatantRect);

	void SetTurnPending(bool _turnPending);

	const sf::IntRect &GetRect();

private:

	CGameEngine *engine;
	CombatantStatus *status;
	Bar healthBar;
	CombatantStatusTooltip tooltip;

	bool isTurnPending;

	void RenderStatusSymbols();
	void RenderStatusSymbol(bool _isActive, CombatantStatus::Type _type, std::unique_ptr<CSprite> &_sprite, int &_x);
	void RenderStatusSymbolsTooltips();
	CombatantStatus::Type GetTooltipStatusType();

	double statusRemoveTime[static_cast<int>(CombatantStatus::Type::number_of_states)];
	double statusAddTime[static_cast<int>(CombatantStatus::Type::number_of_states)];
};
