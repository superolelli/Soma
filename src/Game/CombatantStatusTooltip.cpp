#include "CombatantStatusTooltip.hpp"
#include "Resources/SpritePool.hpp"
#include "Resources/Colors.hpp"

CombatantStatusTooltip::CombatantStatusTooltip()
    : Tooltip()
	, combatantStatus(nullptr)
	, statusType(CombatantStatus::Type::buffed)
{
}

void CombatantStatusTooltip::SetCombatantStatusType(CombatantStatus::Type _type, CombatantStatus *_status)
{
	statusType = _type;
	combatantStatus = _status;
}

void CombatantStatusTooltip::GenerateTooltipString(std::string& _tooltip)
{
	switch (statusType) {
	case CombatantStatus::Type::dynamite:
		_tooltip.append("*" + Colors::WARNING + "Dynamit*\nExplodiert zu 20%\n" + Colors::INFO + "Falls ja: 15 Schaden\nFalls nein: Weiter an Nächsten");
		break;
	case CombatantStatus::Type::sleeping:
		_tooltip.append("Schläft");
		break;
	case CombatantStatus::Type::fist_of_revenge_debuff:
		_tooltip.append(Colors::INFO_HIGHLIGHT + "Angreifbar durch\nFist of Revenge");
		break;
	case CombatantStatus::Type::noface_buff:
		_tooltip.append("*" + Colors::WARNING + "Seine Wunden machen\nihn stärker*\n" + Colors::DEFAULT);
		if (combatantStatus->GetNofaceBuffLevel() > 0)
			AppendCombatantAttributesTooltip(_tooltip, combatantStatus->GetNofaceStats(), " ", "+", Colors::WARNING);
		break;
	case CombatantStatus::Type::decay: 
		_tooltip.append(Colors::WARNING + std::to_string(combatantStatus->GetDecay()) + " Verfall (" + DurationString(combatantStatus->RoundsDecay()) + ")");
		break;
	case CombatantStatus::Type::confused:
		_tooltip.append(Colors::CONFUSION + "Verwirrt (" + DurationString(combatantStatus->RoundsConfused()) + ")");
		break;
	case CombatantStatus::Type::marked:
		_tooltip.append("Markiert (" + DurationString(combatantStatus->RoundsMarked()) + ")");
		break;
	case CombatantStatus::Type::bounty:
		_tooltip.append("Kopfgeld (" + DurationString(combatantStatus->RoundsBounty()) + ")\n" + Colors::INFO + "Bei Schaden:\n+2 Schaden für Angreifer");
		break;
	case CombatantStatus::Type::miss:
		if (combatantStatus->NumberOfMisses() == 1)
			_tooltip.append("1 Fehlschuss ");
		else
			_tooltip.append(std::to_string(combatantStatus->NumberOfMisses()) + "Fehlschüsse ");
		break;
	case CombatantStatus::Type::buffed:
		_tooltip.append(DurationString(combatantStatus->GetBuff().duration) + ":\n");
		AppendCombatantAttributesTooltip(_tooltip, combatantStatus->GetBuff().stats, " ", "+", Colors::BUFF);
		break;
	case CombatantStatus::Type::debuffed:
		_tooltip.append(DurationString(combatantStatus->GetDebuff().duration) + ":\n");
		AppendCombatantAttributesTooltip(_tooltip, combatantStatus->GetDebuff().stats, " ", "", Colors::BUFF);
		break;
	case CombatantStatus::Type::fatigue_tired:
		_tooltip.append("*" + Colors::FATIGUE + "Müde*\n" + Colors::DEFAULT);
		AppendCombatantAttributesTooltip(_tooltip, combatantStatus->fatigueDebuff, " ", "+");
		break;
	case CombatantStatus::Type::fatigue_stupid:
		_tooltip.append(Colors::FATIGUE + " Nach müde kommt doof");
		break;
	}

	if (_tooltip.back() == '\n')
		_tooltip.pop_back();
}