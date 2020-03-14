#pragma once

#include "Resources\ObjectPropertiesManager.hpp"
#include "ItemTooltip.hpp"
#include "EquipmentConnections.hpp"

class EquipmentPanelDiamond
{
public:
	void Init(CGameEngine *_engine, int _xPos, int _yPos);
	void Update();
	void Render();

	void RecolorDiamond(EquipmentConnections _connections[4]);
	CombatantAttributes &GetStats() { return diamondStats.stats; }

private:
	CGameEngine *engine;
	CSprite diamond;

	float recolorTime;
	sf::Color oldColor;
	sf::Color newColor;

	EquipmentProperties diamondStats;
	ItemTooltip tooltip;

	void UpdateStats(EquipmentConnections _connections[4]);
};