#include "LevelGUICommon.hpp"
#include "GameStatus.hpp"


LevelGUICommon::LevelGUICommon(CGameEngine* _engine, LevelStatus *_levelStatus, NotificationRenderer* _notificationRenderer)
	: engine(_engine)
	, notificationRenderer(_notificationRenderer)
	, fatigueBar(g_pTextures->fatigueBar, g_pTextures->fatigueBarFrame, _levelStatus->GetFatiguePtr(), _levelStatus->GetMaxFatiguePtr())
	, resourcesStatusBar(engine)
{
	fatigueBar.SetSmoothTransformationTime(0.7);
	fatigueBar.SetOffsetForInnerPart(6, 15);
	fatigueBar.SetPos(engine->GetWindowSize().x / 2 - fatigueBar.GetRect().width / 2, 3);
	fatigueBar.SetText(g_pFonts->f_kingArthur, sf::Color(200,200,200), 10);
}

void LevelGUICommon::Update()
{
	resourcesStatusBar.Update(g_pGameStatus->GetCardsAmount(), g_pGameStatus->GetDiceAmount());

	auto fatigueChange = fatigueBar.Update(g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds());
	if (fatigueChange != 0)
	{
		auto notificationPos = sf::Vector2f(engine->GetWindowSize().x / 2 + engine->GetViewPosition().x, 20);
		notificationRenderer->AddNotification(std::to_string(fatigueChange), g_pFonts->f_kingArthur, notificationPos, 1.0f, sf::Color(70, 44, 108), sf::Color::Black, 30, false);

	}
}

void LevelGUICommon::Render()
{
	resourcesStatusBar.Render();
	fatigueBar.Render(engine->GetRenderTarget(), true);
}
