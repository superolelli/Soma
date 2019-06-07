#include "SpritePool.hpp"




void SpritePool::LoadSprites()
{
	buff.Load(g_pTextures->buff);
	debuff.Load(g_pTextures->debuff);
	confused.Load(g_pTextures->confused);
	marked.Load(g_pTextures->marked);
	sleeping.Load(g_pTextures->sleeping);

	abilityAnnouncementBanner.Load(g_pTextures->abilityAnnouncementBanner);
	abilityAnnouncementBanner.AddText("Keine Fähigkeit");
	abilityAnnouncementBanner.SetTextFont(0, g_pFonts->f_kingArthur);
	abilityAnnouncementBanner.SetTextCharacterSize(0, 42);
	abilityAnnouncementBanner.SetTextColor(0, sf::Color(220, 220, 220));

	abilityEffectsAnimation = g_pModels->modelAbilityEffects->getNewEntityInstance("ability_effects");
	newBattleAnimation = g_pModels->modelNewBattleAnimation->getNewEntityInstance("New Battle");
	newBattleAnimation->setScale(SpriterEngine::point(0.7, 0.7));

	abilityTargetMarker.Load(g_pTextures->abilityTargetMarker);
	turnMarker.Load(g_pTextures->turnMarker);
}




void SpritePool::FreeSprites()
{
	SAFE_DELETE(abilityEffectsAnimation);
	SAFE_DELETE(newBattleAnimation);
}
