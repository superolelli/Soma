#pragma once


#include "Player.hpp"
#include "PlayerEnum.hpp"


class PlayerOle : public Player
{
public:
	virtual void Init(int _id, CGameEngine *_engine) override;

	virtual int GetID() override { return PlayerID::Ole; }

private:
};

