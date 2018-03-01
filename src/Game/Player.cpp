#include "Player.hpp"





void Player::Init(int _id)
{
	combatantObject->setCurrentAnimation("idle");

	combatantObject->setScale(SpriterEngine::point(0.6, 0.6));
	combatantObject->reprocessCurrentTime();

	SpriterEngine::UniversalObjectInterface* hitboxObj = combatantObject->getObjectInstance("bounding_box");

	hitbox.width = hitboxObj->getSize().x * combatantObject->getScale().x;
	hitbox.height = hitboxObj->getSize().y * combatantObject->getScale().y;
	hitbox.left = hitboxObj->getPosition().x;
	hitbox.top = hitboxObj->getPosition().y;

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
		combatantObject->setTimeElapsed(17);
	else
		combatantObject->setTimeElapsed(10);

	combatantObject->render();
}




