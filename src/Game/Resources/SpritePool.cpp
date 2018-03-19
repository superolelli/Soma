#include "SpritePool.hpp"




void SpritePool::LoadSprites()
{
	buff.Load(g_pTextures->buff);
	debuff.Load(g_pTextures->debuff);
	confused.Load(g_pTextures->confused);
	marked.Load(g_pTextures->marked);
	sleeping.Load(g_pTextures->sleeping);
}
