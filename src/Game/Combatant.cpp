#include "Combatant.hpp"




void Combatant::SetPos(int _x, int _y)
{
	combatantObject->setPosition(SpriterEngine::point(_x, _y));

	combatantObject->reprocessCurrentTime();

	SpriterEngine::UniversalObjectInterface* hitboxObj = combatantObject->getObjectInstance("bounding_box");

	hitbox.left = hitboxObj->getPosition().x;
	hitbox.top = hitboxObj->getPosition().y;

	healthBar.SetPos(GetRect().left + GetRect().width / 2 - healthBar.GetRect().width / 2, GetRect().top + GetRect().height + 30);
}

void Combatant::RenderHealthBar(sf::RenderTarget & _target)
{
	healthBar.Render(_target);
	RenderStatusSymbols(_target);
}


void Combatant::RenderStatusSymbols(sf::RenderTarget & _target)
{
	int x = healthBar.GetRect().left;
	int y = healthBar.GetRect().top + healthBar.GetRect().height;

	if (status.IsAsleep())
	{
		g_pSpritePool->sleeping.SetPos(x, y);
		g_pSpritePool->sleeping.Render(_target);
		x += 20;
	}

	if (status.IsConfused())
	{
		g_pSpritePool->confused.SetPos(x, y);
		g_pSpritePool->confused.Render(_target);
		x += 20;
	}

	if (status.IsBuffed())
	{
		g_pSpritePool->buff.SetPos(x, y);
		g_pSpritePool->buff.Render(_target);
		x += 20;
	}

	if (status.IsDebuffed())
	{
		g_pSpritePool->debuff.SetPos(x, y);
		g_pSpritePool->debuff.Render(_target);
		x += 20;
	}

	if (status.IsMarked())
	{
		g_pSpritePool->marked.SetPos(x, y);
		g_pSpritePool->marked.Render(_target);
	}


}


void Combatant::Quit()
{
	SAFE_DELETE(combatantObject);
}