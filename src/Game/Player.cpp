#include "Player.hpp"





void Player::Init(int _id)
{
	if(_id == 0)
		combatantObject = g_pModels->modelOle->getNewEntityInstance("Ole");
	else if(_id == 1)
		combatantObject = g_pModels->modelAnna->getNewEntityInstance("anna");
	else if (_id == 2)
		combatantObject = g_pModels->modelSimon->getNewEntityInstance("Simon");
	else 
		combatantObject = g_pModels->modelMarkus->getNewEntityInstance("Markus");

	combatantObject->setCurrentAnimation("idle");

	combatantObject->setScale(SpriterEngine::point(0.6, 0.6));
	combatantObject->reprocessCurrentTime();

	SpriterEngine::UniversalObjectInterface* hitboxObj = combatantObject->getObjectInstance("bounding_box");

	hitbox.width = hitboxObj->getSize().x * combatantObject->getScale().x;
	hitbox.height = hitboxObj->getSize().y * combatantObject->getScale().y;
	hitbox.left = hitboxObj->getPosition().x;
	hitbox.top = hitboxObj->getPosition().y;

	attributes.armour = 1;
	attributes.currentHealth = 20;
	attributes.maxHealth = 20;
	attributes.damage = 5;
	attributes.initiative = 1;

	is_walking = false;
}




void Player::Quit()
{
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




