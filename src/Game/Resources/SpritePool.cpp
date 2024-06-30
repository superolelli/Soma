#include "SpritePool.hpp"




void SpritePool::LoadSprites()
{
	buff = std::make_unique<CSprite>(g_pTextures->buff);
	debuff = std::make_unique<CSprite>(g_pTextures->debuff);
	confused = std::make_unique<CSprite>(g_pTextures->confused);
	marked = std::make_unique<CSprite>(g_pTextures->marked);
	sleeping = std::make_unique<CSprite>(g_pTextures->sleeping);
	fistOfRevengeDebuff = std::make_unique<CSprite>(g_pTextures->fistOfRevengeDebuff);
	decay = std::make_unique<CSprite>(g_pTextures->decay);
	fatigue_tired = std::make_unique<CSprite>(g_pTextures->fatigueTired);
	fatigue_stupid = std::make_unique<CSprite>(g_pTextures->fatigueStupid);
	noface_buff = std::make_unique<CSprite>(g_pTextures->nofaceBuff);
	missed = std::make_unique<CSprite>(g_pTextures->missed);
	dynamite = std::make_unique<CSprite>(g_pTextures->dynamite);
	bounty = std::make_unique<CSprite>(g_pTextures->bounty);

	abilityAnnouncementBanner = std::make_unique<SpriteWithText>(g_pTextures->abilityAnnouncementBanner);
	abilityAnnouncementBanner->AddText("Keine Fähigkeit");
	abilityAnnouncementBanner->SetTextFont(0, g_pFonts->f_blackwoodCastle);
	abilityAnnouncementBanner->SetTextCharacterSize(0, 42);
	abilityAnnouncementBanner->SetTextColor(0, sf::Color(220, 220, 220));

	abilityEffectsAnimation = g_pModels->modelAbilityEffects->getNewEntityInstance("ability_effects");
	newBattleAnimation = g_pModels->modelNewBattleAnimation->getNewEntityInstance("New Battle");
	newBattleAnimation->setScale(SpriterEngine::point(0.7, 0.7));

	abilityTargetMarker = std::make_unique<CSprite>(g_pTextures->abilityTargetMarker);
	turnMarker = std::make_unique<CSprite>(g_pTextures->turnMarker);
	turnPendingMarker = std::make_unique<CSprite>(g_pTextures->turnPendingMarker);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < g_pTextures->secondLayerBackgrounds[i].size(); j++)
		{
			backgroundSecondLayers[i].emplace_back(g_pTextures->secondLayerBackgrounds[i][j]);
		}
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < g_pTextures->thirdLayerBackgrounds[i].size(); j++)
		{
			backgroundThirdLayers[i].emplace_back(g_pTextures->thirdLayerBackgrounds[i][j]);
		}
	}

	backgroundFourthLayer[0] = std::make_unique<CSprite>(g_pTextures->fourthLayerBackground[0]);
	backgroundFourthLayer[1] = std::make_unique<CSprite>(g_pTextures->fourthLayerBackground[1]);
}


void SpritePool::FreeSprites()
{
	SAFE_DELETE(abilityEffectsAnimation);
	SAFE_DELETE(newBattleAnimation);
}


bool SpritePool::AbilityEffectIsPlaying()
{
	if (abilityEffectsAnimation->currentAnimationName() == "empty")
		return false;
	else
		return abilityEffectsAnimation->animationIsPlaying();
}