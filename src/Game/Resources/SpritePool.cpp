#include "SpritePool.hpp"




void SpritePool::LoadSprites()
{
	buff.Load(g_pTextures->buff);
	debuff.Load(g_pTextures->debuff);
	confused.Load(g_pTextures->confused);
	marked.Load(g_pTextures->marked);
	sleeping.Load(g_pTextures->sleeping);
	decay.Load(g_pTextures->decay);
	fatigue_tired.Load(g_pTextures->fatigueTired);
	fatigue_stupid.Load(g_pTextures->fatigueStupid);
	noface_buff.Load(g_pTextures->nofaceBuff);
	missed.Load(g_pTextures->missed);

	abilityAnnouncementBanner.Load(g_pTextures->abilityAnnouncementBanner);
	abilityAnnouncementBanner.AddText("Keine Fähigkeit");
	abilityAnnouncementBanner.SetTextFont(0, g_pFonts->f_blackwoodCastle);
	abilityAnnouncementBanner.SetTextCharacterSize(0, 42);
	abilityAnnouncementBanner.SetTextColor(0, sf::Color(220, 220, 220));

	abilityEffectsAnimation = g_pModels->modelAbilityEffects->getNewEntityInstance("ability_effects");
	newBattleAnimation = g_pModels->modelNewBattleAnimation->getNewEntityInstance("New Battle");
	newBattleAnimation->setScale(SpriterEngine::point(0.7, 0.7));

	abilityTargetMarker.Load(g_pTextures->abilityTargetMarker);
	turnMarker.Load(g_pTextures->turnMarker);
	turnPendingMarker.Load(g_pTextures->turnPendingMarker);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < g_pTextures->secondLayerBackgrounds[i].size(); j++)
		{
			backgroundSecondLayers[i].emplace_back(CSprite());
			backgroundSecondLayers[i].back().Load(g_pTextures->secondLayerBackgrounds[i][j]);
		}
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < g_pTextures->thirdLayerBackgrounds[i].size(); j++)
		{
			backgroundThirdLayers[i].emplace_back(CSprite());
			backgroundThirdLayers[i].back().Load(g_pTextures->thirdLayerBackgrounds[i][j]);
		}
	}

	backgroundFourthLayer[0].Load(g_pTextures->fourthLayerBackground[0]);
	backgroundFourthLayer[1].Load(g_pTextures->fourthLayerBackground[1]);
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