#include "EquipmentPanelDiamond.hpp"


void EquipmentPanelDiamond::Init(CGameEngine *_engine, int _xPos, int _yPos)
{
	engine = _engine;
	diamond.Load(g_pTextures->inventoryDiamond);
	diamond.SetPos(_xPos, _yPos);
	diamond.SetColor(0, 0, 0);

	recolorTime = 0.0f;
	diamondStats.stats.Reset();
	diamondStats.level = 0;
	diamondStats.name = "Edelstein der Macht  ";

	tooltip.Init();
	tooltip.SetEquipmentStats(&diamondStats);
}

void EquipmentPanelDiamond::Update()
{
	if (recolorTime > 0.0f)
	{
		float t = (4.0f - recolorTime) / 4.0f;
		recolorTime -= g_pTimer->GetElapsedTime().asSeconds();

		//lerp
		int r = (1.0f - t) * oldColor.r + t * newColor.r;
		int g = (1.0f - t) * oldColor.g + t * newColor.g;
		int b = (1.0f - t) * oldColor.b + t * newColor.b;

		diamond.SetColor(r, g, b);
	}
}

void EquipmentPanelDiamond::Render()
{
	diamond.Render(engine->GetWindow());
	if (diamond.GetGlobalRect().contains(engine->GetMousePos()))
		tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
}


void EquipmentPanelDiamond::RecolorDiamond(EquipmentConnections _connections[4])
{
	recolorTime = 4.0f;

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
	CombatantStats stats;
	stats.Reset();

	if (newColor == sf::Color(64, 0, 0))
		stats.attributes.strength = 1;
	else if (newColor == sf::Color(128, 0, 0))
		stats.attributes.strength = 4;
	else if (newColor == sf::Color(255, 0, 0))
		stats.attributes.strength = 8;
	else if (newColor == sf::Color(0, 64, 0))
		stats.attributes.constitution = 1;
	else if (newColor == sf::Color(0, 128, 0))
		stats.attributes.constitution = 3;
	else if (newColor == sf::Color(0, 255, 0))
		stats.attributes.constitution = 6;
	else if (newColor == sf::Color(0, 0, 64))
		stats.attributes.speed = 1;
	else if (newColor == sf::Color(0, 0, 128))
		stats.attributes.speed = 3;
	else if (newColor == sf::Color(0, 0, 255))
		stats.attributes.speed = 6;
	else if (newColor == sf::Color(64, 64, 0))
	{
		for (int i = 0; i < 4; i++)
		{
			if (_connections[i].GetColor() == sf::Color(128, 128, 0))
			{
				stats.attributes.dexterity = 1;
				break;
			}
			else if (_connections[i].GetColor() == sf::Color(128, 0, 0))
			{
				stats.attributes.strength = 1;
				stats.attributes.constitution = 1;
				break;
			}
		}
	}
	else if (newColor == sf::Color(128, 128, 0))
		stats.attributes.dexterity = 4;
	else if (newColor == sf::Color(255, 255, 0))
		stats.attributes.dexterity = 8;
	else if (newColor == sf::Color(64, 0, 64))
	{
		stats.attributes.strength = 1;
		stats.attributes.speed = 1;
	}
	else if (newColor == sf::Color(128, 64, 0))
	{
		stats.attributes.strength = 1;
		stats.attributes.dexterity = 1;
	}
	else if (newColor == sf::Color(0, 64, 64))
	{
		stats.attributes.constitution = 1;
		stats.attributes.speed = 1;
	}
	else if (newColor == sf::Color(64, 128, 0))
	{
		stats.attributes.dexterity = 1;
		stats.attributes.constitution = 1;
	}
	else if (newColor == sf::Color(64, 64, 64))
	{
		stats.attributes.dexterity = 1;
		stats.attributes.speed = 1;
	}

	diamondStats.stats = stats;
}
