#include "Level.hpp"




void Level::Init()
{
	int backgroundIDs[7] = {corridor_1, corridor_1, door_1, corridor_1, door_2, room_1, end };
	int position = 0;

	for (auto b : backgroundIDs)
	{
		CSprite sprite;
		sprite.Load(g_pTextures->bangBackgrounds[b]);
		sprite.SetPos(position, 0);
		backgrounds.push_back(sprite);

		position += sprite.GetRect().width;
	}
}



void Level::Update()
{

}



void Level::Render(sf::RenderTarget &_target, int _viewX)
{
	int backgroundNr = _viewX / backgrounds[0].GetRect().width;

	if (backgrounds.size() > backgroundNr)
	{
		backgrounds[backgroundNr].Render(_target);

		if(backgrounds.size() > backgroundNr + 1)
			backgrounds[backgroundNr + 1].Render(_target);
	}


}