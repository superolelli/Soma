#pragma once

#include "../Framework/Gui/Bar.hpp"
#include "CombatantStatus.hpp"


class CombatantStatusBar
{
public:

	void Init(CombatantStatus *_status, CGameEngine *_engine);
	void Render();
	void Update(sf::IntRect &_combatantRect);

	const sf::IntRect &GetRect();

private:

	CGameEngine *engine;
	CombatantStatus *status;
	Bar healthBar;

	void RenderStatusSymbols();
	void RenderStatusSymbolsTooltips();
	void RenderBuffTooltip(Buff &_buff, bool _positive);
	void RenderTooltip(const std::string &_tooltip, float _x, float _y);
};
