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



void Combatant::LooseHealth(int _damage)
{
	attributes.currentHealth -= _damage - ((float)attributes.armour/100.0f * _damage);

	if (attributes.currentHealth < 0)
		attributes.currentHealth = 0;
}


void Combatant::GainHealth(int _health)
{
	attributes.currentHealth += _health;

	if (attributes.currentHealth > attributes.maxHealth)
		attributes.currentHealth = attributes.maxHealth;
}

void Combatant::Quit()
{
	SAFE_DELETE(combatantObject);
}

