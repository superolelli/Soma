#include "Player.hpp"





void Player::Init(int _id)
{
	if(_id == 0)
		playerObject = g_pModels->modelOle->getNewEntityInstance("Ole");
	else if(_id == 1)
		playerObject = g_pModels->modelAnna->getNewEntityInstance("anna");
	else if (_id == 2)
		playerObject = g_pModels->modelSimon->getNewEntityInstance("Simon");
	else 
		playerObject = g_pModels->modelMarkus->getNewEntityInstance("Markus");

	playerObject->setCurrentAnimation("idle");

	playerObject->setScale(SpriterEngine::point(0.6, 0.6));
	playerObject->reprocessCurrentTime();

	is_walking = false;
}




void Player::Quit()
{
	SAFE_DELETE(playerObject);
}



void Player::Update(int _xMove, bool _is_walking)
{
	playerObject->setPosition(SpriterEngine::point(playerObject->getPosition().x + _xMove, playerObject->getPosition().y));

	if (is_walking == false && _is_walking == true)
	{
		is_walking = true;
		playerObject->setCurrentAnimation("walk");
	}
	else if (is_walking == true && _is_walking == false)
	{
		is_walking = false;
		playerObject->setCurrentAnimation("idle");
	}

}



void Player::Render()
{
	if(is_walking)
		playerObject->setTimeElapsed(17);
	else
		playerObject->setTimeElapsed(10);

	playerObject->render();
}


void Player::SetPos(int _x, int _y)
{
	playerObject->setPosition(SpriterEngine::point(_x, _y));
}


