#pragma once


#include "../../Framework/Graphics/Sprite.hpp"
#include "TextureManager.hpp"


#define g_pSpritePool SpritePool::Get()
class SpritePool : public TSingleton<SpritePool>
{
public:
	//Loads all textures
	void LoadSprites();

	CSprite buff;
	CSprite debuff;
	CSprite confused;
	CSprite marked;
	CSprite sleeping;
};