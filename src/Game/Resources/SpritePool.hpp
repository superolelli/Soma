#pragma once


#include "../../Framework/Graphics/Sprite.hpp"
#include "../../Framework/Graphics/SpriteWithText.hpp"
#include "../../Framework/Animations/spriterengine.h"
#include "TextureManager.hpp"
#include "FontManager.hpp"
#include "ModelManager.hpp"


#define g_pSpritePool SpritePool::Get()
class SpritePool : public TSingleton<SpritePool>
{
public:
	//Loads all textures
	void LoadSprites();
	void FreeSprites();

	bool AbilityEffectIsPlaying();

	CSprite buff;
	CSprite debuff;
	CSprite confused;
	CSprite marked;
	CSprite sleeping;
	CSprite decay;
	CSprite fatigue_tired;
	CSprite fatigue_stupid;
	CSprite noface_buff;
	CSprite missed;

	SpriteWithText abilityAnnouncementBanner;

	SpriterEngine::EntityInstance *abilityEffectsAnimation;

	SpriterEngine::EntityInstance *newBattleAnimation;

	CSprite abilityTargetMarker;
	CSprite turnMarker;
	CSprite turnPendingMarker;

	std::vector<CSprite> backgroundSecondLayers[2];
	std::vector<CSprite> backgroundThirdLayers[2];
	CSprite backgroundFourthLayer[2];
};