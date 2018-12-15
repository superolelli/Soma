#include "Anna.hpp"



void PlayerAnna::Init(int _id, CGameEngine *_engine)
{
	combatantObject = g_pModels->modelAnna->getNewEntityInstance("anna");

	this->Player::Init(_id, _engine);
}
