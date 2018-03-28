#include "Player.hpp"



void Player::Init(int _id)
{
	combatantObject->setCurrentAnimation("idle");

	combatantObject->setScale(SpriterEngine::point(PLAYER_SCALE, PLAYER_SCALE));
	combatantObject->reprocessCurrentTime();

	possibleAbilityAims[0] = g_pObjectProperties->playerAbilities[_id][0];
	possibleAbilityAims[1] = g_pObjectProperties->playerAbilities[_id][1];
	possibleAbilityAims[2] = g_pObjectProperties->playerAbilities[_id][2];
	possibleAbilityAims[3] = g_pObjectProperties->playerAbilities[_id][3];

	status.SetAttributes(g_pObjectProperties->playerAttributes[_id]);

	SpriterEngine::UniversalObjectInterface* hitboxObj = combatantObject->getObjectInstance("bounding_box");

	hitbox.width = hitboxObj->getSize().x * combatantObject->getScale().x;
	hitbox.height = hitboxObj->getSize().y * combatantObject->getScale().y;
	hitbox.left = hitboxObj->getPosition().x;
	hitbox.top = hitboxObj->getPosition().y;

	healthBar.Load(g_pTextures->healthBar, g_pTextures->healthBarFrame, status.GetCurrentHealthPointer(), status.GetMaxHealthPointer());
	healthBar.SetPos(GetRect().left + GetRect().width / 2 - healthBar.GetRect().width / 2, GetRect().top + GetRect().height + 30);
	healthBar.SetSmoothTransformationTime(0.7f);

	status.Reset();

	is_walking = false;
}




void Player::Update(int _xMove, bool _is_walking)
{
	SetPos(combatantObject->getPosition().x + _xMove, combatantObject->getPosition().y);

	if (is_walking == false && _is_walking == true)
	{
		is_walking = true;
		combatantObject->setCurrentAnimation("walk");
	}
	else if (is_walking == true && _is_walking == false)
	{
		is_walking = false;
		combatantObject->setCurrentAnimation("idle");
	}
}



void Player::Render()
{
	if(is_walking)
		combatantObject->setTimeElapsed(WALKING_ANIMATION_SPEED);
	else
		combatantObject->setTimeElapsed(IDLE_ANIMATION_SPEED);

	combatantObject->render();
}




