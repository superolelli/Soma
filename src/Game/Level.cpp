#include "Level.hpp"




void Level::Init()
{
	int backgroundIDs[7] = {corridor_1, corridor_1, door_1, corridor_1, door_2, room_1, end };
	int battlesTemp[14] = { 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 };
	int position = 0;
	battle = false;

	for (auto b : backgroundIDs)
	{
		CSprite sprite;
		sprite.Load(g_pTextures->bangBackgrounds[b]);
		sprite.SetPos(position, 0);
		backgrounds.push_back(sprite);

		position += sprite.GetRect().width;
	}

	for (int i= 0; i < 10; i++)
	{
		battles[i] = battlesTemp[i];
	}
}



void Level::Update(int _playerPos)
{
	battle = false;

	if (battles[_playerPos / (backgrounds[0].GetRect().width / 2)] == 1)
	{
		battle = true;
		battles[_playerPos / (backgrounds[0].GetRect().width / 2)] = 0;
	}
}



void Level::Render(sf::RenderTarget &_target, int _viewX)
{
	RenderBackground(_target, _viewX);
}



void Level::RenderBackground(sf::RenderTarget &_target, int _viewX)
{
	int backgroundNr = _viewX / backgrounds[0].GetRect().width;

	if (backgrounds.size() > backgroundNr)
	{
		backgrounds[backgroundNr].Render(_target);

		if (backgrounds.size() > backgroundNr + 1)
			backgrounds[backgroundNr + 1].Render(_target);
	}
}