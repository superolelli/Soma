#include "SpritePool.hpp"




void SpritePool::LoadSprites()
{
	buff.Load(g_pTextures->buff);
	debuff.Load(g_pTextures->debuff);
	confused.Load(g_pTextures->confused);
	marked.Load(g_pTextures->marked);
	sleeping.Load(g_pTextures->sleeping);

	abilityAnnouncementBanner.Load(g_pTextures->abilityAnnouncementBanner);
	abilityAnnouncementBanner.AddText("No ability");
	abilityAnnouncementBanner.SetTextFont(0, g_pFonts->f_kingArthur);
	abilityAnnouncementBanner.SetTextCharacterSize(0, 28);
	abilityAnnouncementBanner.SetTextColor(0, sf::Color(220, 220, 220));
}
