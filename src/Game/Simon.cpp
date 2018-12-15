#include "Simon.hpp"



void PlayerSimon::Init(int _id, CGameEngine *_engine)
{
	combatantObject = g_pModels->modelSimon->getNewEntityInstance("Simon");

	this->Player::Init(_id, _engine);
}
