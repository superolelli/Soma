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
	CSprite damageOverTime;
	CSprite fatigue_tired;
	CSprite fatigue_stupid;
	CSprite noface_buff;

	SpriteWithText abilityAnnouncementBanner;

	SpriterEngine::EntityInstance *abilityEffectsAnimation;

	SpriterEngine::EntityInstance *newBattleAnimation;

	CSprite abilityTargetMarker;
	CSprite turnMarker;
	CSprite turnPendingMarker;

	CSprite bangBackgroundSecondLayers[NUMBER_OF_SECOND_LAYER_BACKGROUNDS];
	CSprite bangBackgroundThirdLayers[NUMBER_OF_THIRD_LAYER_BACKGROUNDS];
	CSprite bangBackgroundFourthLayer;
};