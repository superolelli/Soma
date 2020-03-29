#include "CombatantStatusBar.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\SpritePool.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"




void CombatantStatusBar::Init(CombatantStatus *_status, CGameEngine *_engine)
{
	engine = _engine;
	status = _status;
	healthBar.Load(g_pTextures->healthBar, g_pTextures->healthBarFrame, status->GetCurrentHealthPointer(), status->GetMaxHealthPointer());
	healthBar.SetSmoothTransformationTime(0.7f);

	for (auto &s : statusRemoveTime)
		s = -1.0f;

	for (auto &s : statusAddTime)
		s = -1.0f;
}


const sf::IntRect &CombatantStatusBar::GetRect()
{
	return healthBar.GetRect();
}



void CombatantStatusBar::Update(sf::IntRect &_combatantRect)
{
	healthBar.SetPos(_combatantRect.left + _combatantRect.width / 2 - healthBar.GetRect().width / 2, _combatantRect.top + _combatantRect.height + 30);
	healthBar.Update(g_pTimer->GetElapsedTime().asSeconds());
}


void CombatantStatusBar::Render()
{
	healthBar.Render(engine->GetWindow());
	RenderStatusSymbols();
}


void CombatantStatusBar::RenderStatusSymbols()
{
	int x = healthBar.GetRect().left;
	int y = healthBar.GetRect().top + healthBar.GetRect().height;

	RenderStatusSymbol(status->IsAsleep(), sleeping, g_pSpritePool->sleeping, x);
	RenderStatusSymbol(status->DamageOverTime() > 0.0f, damageOverTime, g_pSpritePool->damageOverTime, x);
	RenderStatusSymbol(status->IsConfused(), confused, g_pSpritePool->confused, x);
	RenderStatusSymbol(status->IsBuffed(), buffed, g_pSpritePool->buff, x);
	RenderStatusSymbol(status->IsDebuffed(), debuffed, g_pSpritePool->debuff, x);
	RenderStatusSymbol(status->IsMarked(), marked, g_pSpritePool->marked, x);

	RenderStatusSymbolsTooltips();
}

void CombatantStatusBar::RenderStatusSymbol(bool _isActive, statusType _type, CSprite & _sprite, int & _x)
{
	int y = healthBar.GetRect().top + healthBar.GetRect().height;

	if (_isActive && statusAddTime[_type] == -1.0f)
	{
		statusAddTime[_type] = 0.5f;
		statusRemoveTime[_type] = 0.0f;
	}
	else if (!_isActive && statusAddTime[_type] > -1.0f)
	{
		statusRemoveTime[_type] = 0.5f;
		statusAddTime[_type] = -1.0f;
	}

	_sprite.SetScale(1.0f, 1.0f);
	if (statusAddTime[_type] > 0.0f)
	{
		float t = 1.0 - statusAddTime[_type] * 2.0;
		float scaleFactor = std::pow((1.0 - t), 3) * 1.5 + 3.0 * t*std::pow((1.0 - t), 2) * 0.64 + 3.0 * t*t* (1.0 - t)*0.635 + t*t*t; //cubic bezier for popping in
		_sprite.SetScale(scaleFactor, scaleFactor);
		statusAddTime[_type] -= g_pTimer->GetElapsedTime().asSeconds();
	}
	else if (statusRemoveTime[_type] > 0.0f)
	{
		float scaleFactor = -2.4 * std::pow(1.0 - (2.0 * statusRemoveTime[_type]), 2.0) + 1.4 * (1.0 - (2.0 * statusRemoveTime[_type])) + 1.0; //quadratic curve for popping out
		_sprite.SetScale(scaleFactor, scaleFactor);
		statusRemoveTime[_type] -= g_pTimer->GetElapsedTime().asSeconds();
	}

	if (_isActive || statusRemoveTime[_type] > 0.0f)
	{
		_sprite.SetPos(_x, y);
		_sprite.Render(engine->GetWindow());
		_x += 20;
	}
}


void CombatantStatusBar::RenderStatusSymbolsTooltips()
{
	if (status->IsAsleep() && g_pSpritePool->sleeping.GetRect().contains(engine->GetWorldMousePos()))
		RenderTooltip("Schläft", g_pSpritePool->sleeping.GetRect().left, g_pSpritePool->sleeping.GetRect().top);

	if (status->RoundsDamageOverTime() > 0 && g_pSpritePool->damageOverTime.GetRect().contains(engine->GetWorldMousePos()))
	{
		if(status->RoundsDamageOverTime() > 1)
			RenderTooltip("#dd3333 " + std::to_string(status->DamageOverTime()) + " Schaden (" + std::to_string(status->RoundsDamageOverTime()) + " Runden)", g_pSpritePool->damageOverTime.GetRect().left, g_pSpritePool->damageOverTime.GetRect().top);
		else
			RenderTooltip("#dd3333 " + std::to_string(status->DamageOverTime()) + " Schaden (1 Runde)", g_pSpritePool->damageOverTime.GetRect().left, g_pSpritePool->damageOverTime.GetRect().top);
	}


	if (status->IsConfused() && g_pSpritePool->confused.GetRect().contains(engine->GetWorldMousePos()))
	{
		if (status->RoundsConfused() > 1)
			RenderTooltip("#bb77bb Verwirrt (" + std::to_string(status->RoundsConfused()) + " Runden)", g_pSpritePool->confused.GetRect().left, g_pSpritePool->confused.GetRect().top);
		else
			RenderTooltip("#bb77bb Verwirrt (" + std::to_string(status->RoundsConfused()) + " Runde)", g_pSpritePool->confused.GetRect().left, g_pSpritePool->confused.GetRect().top);
	}

	if (status->IsMarked() && g_pSpritePool->marked.GetRect().contains(engine->GetWorldMousePos()))
	{
		if (status->RoundsMarked() > 1)
			RenderTooltip("Markiert (" + std::to_string(status->RoundsMarked()) + " Runden)", g_pSpritePool->marked.GetRect().left, g_pSpritePool->marked.GetRect().top);
		else
			RenderTooltip("Markiert (" + std::to_string(status->RoundsMarked()) + " Runde)", g_pSpritePool->marked.GetRect().left, g_pSpritePool->marked.GetRect().top);
	}

	if (status->IsBuffed() && g_pSpritePool->buff.GetRect().contains(engine->GetWorldMousePos()))
		RenderBuffTooltip(status->GetBuff(), true);
	
	if (status->IsDebuffed() && g_pSpritePool->debuff.GetRect().contains(engine->GetWorldMousePos()))
		RenderBuffTooltip(status->GetDebuff(), false);
}


void CombatantStatusBar::RenderBuffTooltip(Buff &_buff, bool _positive)
{
	std::string prefix(" ");

	if (_positive)
		prefix = " +";

	std::string tooltip("");

	if (_buff.duration > 1)
		tooltip.append(std::to_string(_buff.duration) + " Runden:\n#aaaadd ");
	else
		tooltip.append(std::to_string(_buff.duration) + " Runde:\n#aaaadd ");

	if (_buff.stats.armour != 0)
		tooltip.append(prefix + std::to_string(_buff.stats.armour) + " Rüstung\n");
	if (_buff.stats.maxHealth != 0)
		tooltip.append(prefix + std::to_string(_buff.stats.maxHealth) + " Maximales Leben\n");
	if (_buff.stats.damageMin != 0)
		tooltip.append(prefix + std::to_string(_buff.stats.damageMin) + " Schaden\n");
	if (_buff.stats.initiative != 0)
		tooltip.append(prefix + std::to_string(_buff.stats.initiative) + " Initiative\n");
	if (_buff.stats.criticalHit != 0)
		tooltip.append(prefix + std::to_string(_buff.stats.criticalHit) + " Kritische Trefferchance\n");
	if (_buff.stats.dodge != 0)
		tooltip.append(prefix + std::to_string(_buff.stats.dodge) + " Ausweichen\n");
	if (_buff.stats.precision != 0)
		tooltip.append(prefix + std::to_string(_buff.stats.precision) + " Präzision\n");

	tooltip.pop_back();

	if(_positive)
		RenderTooltip(tooltip, g_pSpritePool->buff.GetRect().left, g_pSpritePool->buff.GetRect().top);
	else
		RenderTooltip(tooltip, g_pSpritePool->debuff.GetRect().left, g_pSpritePool->debuff.GetRect().top);
}



void CombatantStatusBar::RenderTooltip(const std::string &_tooltip, float _x, float _y)
{
	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(_tooltip);
	tooltip.setPosition(_x, _y - tooltip.getLocalBounds().height - 30.0f);

	sf::FloatRect backgroundRect = tooltip.getLocalBounds();
	sf::RoundedRectangleShape background(sf::Vector2f(backgroundRect.width + 10.0f, backgroundRect.height + 16.0f), 8, 10);
	background.setFillColor(sf::Color(0, 0, 0, 200));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(60, 60, 60));
	background.setPosition(tooltip.getPosition() + sf::Vector2f(-5.0f, -4.0f));

	engine->GetWindow().draw(background);
	engine->GetWindow().draw(tooltip);
}
