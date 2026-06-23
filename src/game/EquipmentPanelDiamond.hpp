#pragma once

#include "Resources\ObjectPropertiesManager.hpp"
#include "ItemTooltip.hpp"
#include "EquipmentConnections.hpp"

class EquipmentPanelDiamond
{
public:

	EquipmentPanelDiamond(CGameEngine* _engine);
	void Update();
	void Render();

	void SetPos(int _xPos, int _yPos);

	void RecolorDiamond(EquipmentConnections _connections[4]);
	CombatantAttributes &GetStats() { return diamondStats.stats; }

private:
	CGameEngine *engine;
	CSprite diamond;

	double recolorTime;
	sf::Color oldColor;
	sf::Color newColor;

	EquipmentProperties diamondStats;
	ItemTooltip tooltip;

	void UpdateStats(EquipmentConnections _connections[4]);
};