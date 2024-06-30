#pragma once
#include "Tooltip.hpp"
#include "CombatantStatus.hpp"

class CombatantStatusTooltip : public Tooltip
{
public:

	CombatantStatusTooltip();

	void SetCombatantStatusType(CombatantStatus::Type _type, CombatantStatus *_status);

private:
	CombatantStatus::Type statusType;
	CombatantStatus* combatantStatus;

	void GenerateTooltipString(std::string& _tooltip) override;
};