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

	enum statusType {sleeping, damageOverTime, confused, buffed, debuffed, marked};

	CGameEngine *engine;
	CombatantStatus *status;
	Bar healthBar;

	void RenderStatusSymbols();
	void RenderStatusSymbol(bool _isActive, statusType _type, CSprite &_sprite, int &_x);
	void RenderStatusSymbolsTooltips();
	void RenderBuffTooltip(Buff &_buff, bool _positive);
	void RenderTooltip(const std::string &_tooltip, float _x, float _y);

	float statusRemoveTime[6];
	float statusAddTime[6];
};
