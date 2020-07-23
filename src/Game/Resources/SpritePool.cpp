#include "SpritePool.hpp"




void SpritePool::LoadSprites()
{
	buff.Load(g_pTextures->buff);
	debuff.Load(g_pTextures->debuff);
	confused.Load(g_pTextures->confused);
	marked.Load(g_pTextures->marked);
	sleeping.Load(g_pTextures->sleeping);
	damageOverTime.Load(g_pTextures->damageOverTime);
	fatigue_tired.Load(g_pTextures->fatigueTired);
	fatigue_stupid.Load(g_pTextures->fatigueStupid);
	noface_buff.Load(g_pTextures->nofaceBuff);

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

	for(int i = 0; i < NUMBER_OF_SECOND_LAYER_BACKGROUNDS; i++)
		bangBackgroundSecondLayers[i].Load(g_pTextures->bangSecondLayerBackgrounds[i]);

	for (int i = 0; i < NUMBER_OF_THIRD_LAYER_BACKGROUNDS; i++)
		bangBackgroundThirdLayers[i].Load(g_pTextures->bangThirdLayerBackgrounds[i]);

	bangBackgroundFourthLayer.Load(g_pTextures->bangFourthLayerBackground);
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