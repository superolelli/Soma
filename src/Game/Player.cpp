#include "Player.hpp"





void Player::Init()
{
	playerObject = g_pModels->modelOle->getNewEntityInstance("Ole");

	playerObject->setCurrentAnimation("idle");

	playerObject->setScale(SpriterEngine::point(-0.8, 0.8));
	playerObject->reprocessCurrentTime();
}




void Player::Quit()
{
	SAFE_DELETE(playerObject);
}



void Player::Update()
{
}



void Player::Render()
{
	playerObject->setTimeElapsed(10);
	playerObject->render();
}


void Player::SetPos(int _x, int _y)
{
	playerObject->setPosition(SpriterEngine::point(_x, _y));
}