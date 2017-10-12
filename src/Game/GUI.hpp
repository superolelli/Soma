#pragma once

#include "../Framework/Gameengine.hpp"

class GUI
{
public:

	virtual void Init(CGameEngine *_engine) = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

protected:
	CGameEngine *engine;
};