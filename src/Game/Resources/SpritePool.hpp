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

	std::unique_ptr<CSprite> buff;
	std::unique_ptr<CSprite> debuff;
	std::unique_ptr<CSprite> confused;
	std::unique_ptr<CSprite> marked;
	std::unique_ptr<CSprite> sleeping;
	std::unique_ptr<CSprite> fistOfRevengeDebuff;
	std::unique_ptr<CSprite> decay;
	std::unique_ptr<CSprite> fatigue_tired;
	std::unique_ptr<CSprite> fatigue_stupid;
	std::unique_ptr<CSprite> noface_buff;
	std::unique_ptr<CSprite> missed;
	std::unique_ptr<CSprite> dynamite;
	std::unique_ptr<CSprite> bounty;
						   
	std::unique_ptr<SpriteWithText> abilityAnnouncementBanner;

	SpriterEngine::EntityInstance *abilityEffectsAnimation;

	SpriterEngine::EntityInstance *newBattleAnimation;

	std::unique_ptr<CSprite> abilityTargetMarker;
	std::unique_ptr<CSprite> turnMarker;
	std::unique_ptr<CSprite> turnPendingMarker;

	std::vector<CSprite> backgroundSecondLayers[2];
	std::vector<CSprite> backgroundThirdLayers[2];
	std::unique_ptr<CSprite> backgroundFourthLayer[2];
};