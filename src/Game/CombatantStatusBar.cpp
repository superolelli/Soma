#include "CombatantStatusBar.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\SpritePool.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include <deque>



CombatantStatusBar::CombatantStatusBar(CombatantStatus *_status, CGameEngine *_engine)
	: engine(_engine)
	, status(_status)
	, healthBar(g_pTextures->healthBar, g_pTextures->healthBarFrame, status->GetCurrentHealthPointer(), status->GetMaxHealthPointer())
	, isTurnPending(false)
{
	healthBar.SetSmoothTransformationTime(0.7);

	for (int i = 0; i < static_cast<int>(CombatantStatus::Type::number_of_states); i++)
	{
		statusRemoveTime[i] = -1.0;
		statusAddTime[i] = -1.0;
	}
}


const sf::IntRect &CombatantStatusBar::GetRect()
{
	return healthBar.GetRect();
}



void CombatantStatusBar::Update(sf::IntRect &_combatantRect)
{
	healthBar.SetPos(_combatantRect.left + _combatantRect.width / 2 - healthBar.GetRect().width / 2, _combatantRect.top + _combatantRect.height + 30);
	healthBar.Update(g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds());
}

void CombatantStatusBar::SetTurnPending(bool _turnPending)
{
	isTurnPending = _turnPending;
}


void CombatantStatusBar::Render()
{
	healthBar.Render(engine->GetRenderTarget());
	RenderStatusSymbols();

	if (isTurnPending)
	{
		g_pSpritePool->turnPendingMarker->SetPos(healthBar.GetRect().left + healthBar.GetRect().width / 2 - g_pSpritePool->turnPendingMarker->GetRect().width / 2, healthBar.GetRect().top - g_pSpritePool->turnPendingMarker->GetRect().height - 2);
		g_pSpritePool->turnPendingMarker->Render(engine->GetRenderTarget());
	}
}


void CombatantStatusBar::RenderStatusSymbols()
{
	int x = healthBar.GetRect().left;
	int y = healthBar.GetRect().top + healthBar.GetRect().height;

	RenderStatusSymbol(status->HasDynamite(), CombatantStatus::Type::dynamite, g_pSpritePool->dynamite, x);
	RenderStatusSymbol(status->IsAsleep(), CombatantStatus::Type::sleeping, g_pSpritePool->sleeping, x);
	RenderStatusSymbol(status->HasFistOfRevengeDebuff(), CombatantStatus::Type::fist_of_revenge_debuff, g_pSpritePool->fistOfRevengeDebuff, x);
	RenderStatusSymbol(status->GetNofaceBuffLevel() >= 0, CombatantStatus::Type::noface_buff, g_pSpritePool->noface_buff, x);
	RenderStatusSymbol(status->GetFatigueLevel() == CombatantStatus::FatigueLevel::tired, CombatantStatus::Type::fatigue_tired, g_pSpritePool->fatigue_tired, x);
	RenderStatusSymbol(status->GetFatigueLevel() == CombatantStatus::FatigueLevel::stupid, CombatantStatus::Type::fatigue_stupid, g_pSpritePool->fatigue_stupid, x);
	RenderStatusSymbol(status->GetDecay() > 0.0f, CombatantStatus::Type::decay, g_pSpritePool->decay, x);
	RenderStatusSymbol(status->IsConfused(), CombatantStatus::Type::confused, g_pSpritePool->confused, x);
	RenderStatusSymbol(status->IsBuffed(), CombatantStatus::Type::buffed, g_pSpritePool->buff, x);
	RenderStatusSymbol(status->IsDebuffed(), CombatantStatus::Type::debuffed, g_pSpritePool->debuff, x);
	RenderStatusSymbol(status->IsMarked(), CombatantStatus::Type::marked, g_pSpritePool->marked, x);
	RenderStatusSymbol(status->NumberOfMisses() > 0, CombatantStatus::Type::miss, g_pSpritePool->missed, x);
	RenderStatusSymbol(status->HasBounty(), CombatantStatus::Type::bounty, g_pSpritePool->bounty, x);

	RenderStatusSymbolsTooltips();
}

void CombatantStatusBar::RenderStatusSymbol(bool _isActive, CombatantStatus::Type _type, std::unique_ptr<CSprite> & _sprite, int & _x)
{
	int type = static_cast<int>(_type);

	int y = healthBar.GetRect().top + healthBar.GetRect().height;

	if (_isActive && statusAddTime[type] == -1.0)
	{
		statusAddTime[type] = 0.5;
		statusRemoveTime[type] = 0.0;
	}
	else if (!_isActive && statusAddTime[type] > -1.0)
	{
		statusRemoveTime[type] = 0.5;
		statusAddTime[type] = -1.0;
	}

	_sprite->SetScale(1.0f, 1.0f);
	if (statusAddTime[type] > 0.0)
	{
		float t = 1.0 - statusAddTime[type] * 2.0;
		float scaleFactor = std::pow((1.0 - t), 3) * 1.5 + 3.0 * t*std::pow((1.0 - t), 2) * 0.64 + 3.0 * t*t* (1.0 - t)*0.635 + t*t*t; //cubic bezier for popping in
		_sprite->SetScale(scaleFactor, scaleFactor);
		statusAddTime[type] -= g_pTimer->GetElapsedTimeAsSeconds();
	}
	else if (statusRemoveTime[type] > 0.0)
	{
		float scaleFactor = -2.4 * std::pow(1.0 - (2.0 * statusRemoveTime[type]), 2.0) + 1.4 * (1.0 - (2.0 * statusRemoveTime[type])) + 1.0; //quadratic curve for popping out
		_sprite->SetScale(scaleFactor, scaleFactor);
		statusRemoveTime[type] -= g_pTimer->GetElapsedTimeAsSeconds();
	}

	if (_isActive || statusRemoveTime[type] > 0.0)
	{
		_sprite->SetPos(_x, y);
		_sprite->Render(engine->GetRenderTarget());
		_x += 20;
	}
}

CombatantStatus::Type CombatantStatusBar::GetTooltipStatusType()
{
	if (status->HasDynamite() && g_pSpritePool->dynamite->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::dynamite;

	if (status->IsAsleep() && g_pSpritePool->sleeping->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::sleeping;

	if (status->HasFistOfRevengeDebuff() && g_pSpritePool->fistOfRevengeDebuff->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::fist_of_revenge_debuff;

	if (status->GetNofaceBuffLevel() >= 0 && g_pSpritePool->noface_buff->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::noface_buff;

	if (status->RoundsDecay() > 0 && g_pSpritePool->decay->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::decay;

	if (status->IsConfused() && g_pSpritePool->confused->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::confused;

	if (status->IsMarked() && g_pSpritePool->marked->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::marked;

	if (status->HasBounty() && g_pSpritePool->bounty->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::bounty;

	if (status->NumberOfMisses() > 0 && g_pSpritePool->missed->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::miss;

	if (status->IsBuffed() && g_pSpritePool->buff->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::buffed;

	if (status->IsDebuffed() && g_pSpritePool->debuff->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::debuffed;

	if (status->GetFatigueLevel() == CombatantStatus::FatigueLevel::tired && g_pSpritePool->fatigue_tired->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::fatigue_tired;

	if (status->GetFatigueLevel() == CombatantStatus::FatigueLevel::stupid && g_pSpritePool->fatigue_stupid->GetRect().contains(engine->GetWorldMousePos()))
		return CombatantStatus::Type::fatigue_stupid;

	return CombatantStatus::Type::number_of_states;
}

void CombatantStatusBar::RenderStatusSymbolsTooltips()
{
	auto type = GetTooltipStatusType();
	if (type != CombatantStatus::Type::number_of_states) {
		tooltip.SetCombatantStatusType(type, status);
		tooltip.ShowTooltip(engine->GetRenderTarget(), healthBar.GetRect().left, healthBar.GetRect().top - 30);
	}
}