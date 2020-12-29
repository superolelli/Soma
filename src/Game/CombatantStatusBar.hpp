#pragma once

#include "../Framework/Gui/Bar.hpp"
#include "CombatantStatus.hpp"


class CombatantStatusBar
{
public:

	void Init(CombatantStatus *_status, CGameEngine *_engine);
	void Render();
	void Update(sf::IntRect &_combatantRect);

	void SetTurnPending(bool _turnPending);

	const sf::IntRect &GetRect();

private:

	enum statusType {sleeping, decay, confused, buffed, debuffed, marked, fatigue_tired, fatigue_stupid, noface_buff, miss, dynamite, number_of_states};

	CGameEngine *engine;
	CombatantStatus *status;
	Bar healthBar;

	bool isTurnPending;

	void RenderStatusSymbols();
	void RenderStatusSymbol(bool _isActive, statusType _type, CSprite &_sprite, int &_x);
	void RenderStatusSymbolsTooltips();
	void RenderBuffTooltip(Buff &_buff, bool _positive);
	void RenderTooltip(const std::string &_tooltip, float _x, float _y);

	void AddStatsToTooltip(std::string& _tooltip, const std::string& _prefix, const CombatantAttributes &_stats);

	double statusRemoveTime[statusType::number_of_states];
	double statusAddTime[statusType::number_of_states];
};
