#pragma once

#include "GUI.hpp"

class LevelGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine);
	virtual void Update() override;
	virtual void Render() override;

private:
	CGameEngine *engine;

};