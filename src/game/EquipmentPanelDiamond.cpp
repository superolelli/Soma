#include "EquipmentPanelDiamond.hpp"


EquipmentPanelDiamond::EquipmentPanelDiamond(CGameEngine *_engine)
	: engine(_engine)
	, diamond(g_pTextures->inventoryDiamond)
	, recolorTime(0.0)
{
	diamond.SetColor(0, 0, 0);

	diamondStats.stats.Reset();
	diamondStats.level = 0;
	diamondStats.name = "Edelstein der Macht  ";

	tooltip.SetEquipmentStats(&diamondStats);
}

void EquipmentPanelDiamond::SetPos(int _xPos, int _yPos)
{
	diamond.SetPos(_xPos, _yPos);
}

void EquipmentPanelDiamond::Update()
{
	if (recolorTime >= 0.0)
	{
		recolorTime -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
		recolorTime = std::max(0.0, recolorTime);
		double t = std::clamp((3.0 - recolorTime) / 3.0, 0.0, 1.0);
		
		//lerp
		int r = (1.0 - t) * oldColor.r + t * newColor.r;
		int g = (1.0 - t) * oldColor.g + t * newColor.g;
		int b = (1.0 - t) * oldColor.b + t * newColor.b;

		diamond.SetColor(r, g, b);
	}
}

void EquipmentPanelDiamond::Render()
{
	diamond.Render(engine->GetRenderTarget());
	if (diamond.GetGlobalRect().contains(engine->GetMousePos()))
		tooltip.ShowTooltip(engine->GetRenderTarget(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
}


void EquipmentPanelDiamond::RecolorDiamond(EquipmentConnections _connections[4])
{
	recolorTime = 3.0;

	int r = 0;
	int g = 0;
	int b = 0;

	for (int i = 0; i < 4; i++)
	{
		r = std::min(255, r + _connections[i].GetColor().r / 2);
		g = std::min(255, g + _connections[i].GetColor().g / 2);
		b = std::min(255, b + _connections[i].GetColor().b / 2);
	}

	oldColor = diamond.GetColor();
	newColor = sf::Color(r, g, b);

	UpdateStats(_connections);
}



void EquipmentPanelDiamond::UpdateStats(EquipmentConnections _connections[4])
{
	CombatantAttributes stats;
	stats.Reset();

	if (newColor == sf::Color(64, 0, 0))
	{
		stats["damageMax"] = 2;
		stats["damageMin"] = 2;
	}
	else if (newColor == sf::Color(128, 0, 0))
	{
		stats["damageMax"] = 8;
		stats["damageMin"] = 8;
	}
	else if (newColor == sf::Color(255, 0, 0))
	{
		stats["damageMax"] = 16;
		stats["damageMin"] = 16;
	}
	else if (newColor == sf::Color(0, 64, 0))
	{
		stats["maxHealth"] = 5;
		stats["currentHealth"] = 5;
	}
	else if (newColor == sf::Color(0, 128, 0))
	{
		stats["maxHealth"] = 15;
		stats["currentHealth"] = 15;
	}
	else if (newColor == sf::Color(0, 255, 0))
	{
		stats["maxHealth"] = 30;
		stats["currentHealth"] = 30;
	}
	else if (newColor == sf::Color(0, 0, 64))
	{
		stats["dodge"] = 3;
		stats["initiative"] = 1;
	}
	else if (newColor == sf::Color(0, 0, 128))
	{
		stats["dodge"] = 9;
		stats["initiative"] = 3;
	}
	else if (newColor == sf::Color(0, 0, 255))
	{
		stats["dodge"] = 18;
		stats["initiative"] = 6;
	}
	else if (newColor == sf::Color(64, 64, 0))
	{
		for (int i = 0; i < 4; i++)
		{
			if (_connections[i].GetColor() == sf::Color(128, 128, 0))
			{
				stats["precision"] = 1;
				stats["criticalHit"] = 1;
				break;
			}
			else if (_connections[i].GetColor() == sf::Color(128, 0, 0))
			{
				stats["damageMax"] = 2;
				stats["damageMin"] = 2;
				stats["maxHealth"] = 5;
				stats["currentHealth"] = 5;
				break;
			}
		}
	}
	else if (newColor == sf::Color(128, 128, 0))
	{
		stats["criticalHit"] = 4;
		stats["precision"] = 4;
	}
	else if (newColor == sf::Color(255, 255, 0))
	{
		stats["criticalHit"] = 8;
		stats["precision"] = 8;
	}
	else if (newColor == sf::Color(64, 0, 64))
	{
		stats["damageMax"] = 2;
		stats["damageMin"] = 2;
		stats["initiative"] = 1;
		stats["dodge"] = 3;
	}
	else if (newColor == sf::Color(128, 64, 0))
	{
		stats["damageMax"] = 2;
		stats["damageMin"] = 2;
		stats["initiative"] = 1;
		stats["dodge"] = 3;
	}
	else if (newColor == sf::Color(0, 64, 64))
	{
		stats["maxHealth"] = 5;
		stats["currentHealth"] = 5;
		stats["initiative"] = 1;
		stats["dodge"] = 3;
	}
	else if (newColor == sf::Color(64, 128, 0))
	{
		stats["precision"] = 1;
		stats["criticalHit"] = 1;
		stats["maxHealth"] = 5;
		stats["currentHealth"] = 5;
	}
	else if (newColor == sf::Color(64, 64, 64))
	{
		stats["precision"] = 1;
		stats["criticalHit"] = 1;
		stats["initiative"] = 1;
		stats["dodge"] = 3;
	}

	diamondStats.stats = stats;
}
