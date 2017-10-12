#pragma once

#include "GUI.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "Resources\Resources.hpp"

class BattleGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine) override;
	virtual void Update() override;
	virtual void Render() override;

	int GetCurrentAbility() { return currentAbility; }

private:

	int currentAbility;

	CSprite abilities[4];
	CSprite currentAbilityFrame;
};