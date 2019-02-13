#pragma once

#include "GUI.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "Resources\Resources.hpp"
#include "PlayerEnum.hpp"


class BattleGUI : public GUI
{
public:

	virtual void Init(CGameEngine *_engine) override;
	virtual void Update() override;
	virtual void Render() override;

	int GetCurrentAbility() { return currentAbility; }
	void SetCurrentPlayer(int _player);

private:

	int currentAbility;
	PlayerID currentPlayer;

	CSprite abilities[4][4];

	CSprite currentAbilityFrame;

	void ShowTooltip(int _ability);
};