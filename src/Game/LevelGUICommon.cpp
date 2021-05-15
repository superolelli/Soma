#include "LevelGUICommon.hpp"
#include "GameStatus.hpp"


void LevelGUICommon::Init(CGameEngine* _engine, GameStatus *_gameStatus)
{
	engine = _engine;
	gameStatus = _gameStatus;

	fatigueBar.Load(g_pTextures->fatigueBar, g_pTextures->fatigueBarFrame, _gameStatus->GetFatiguePtr(), _gameStatus->GetMaxFatiguePtr());
	fatigueBar.SetSmoothTransformationTime(0.7);
	fatigueBar.SetOffsetForInnerPart(6, 15);
	fatigueBar.SetPos(engine->GetWindowSize().x / 2 - fatigueBar.GetRect().width / 2, 3);
	fatigueBar.SetText(g_pFonts->f_kingArthur, sf::Color(200,200,200), 10);

	resourcesStatusBar.Init(engine);
}

void LevelGUICommon::Update()
{
	resourcesStatusBar.Update(gameStatus->GetCardsAmount(), gameStatus->GetDiceAmount());
	fatigueBar.Update(g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds());
}

void LevelGUICommon::Render()
{
	resourcesStatusBar.Render();
	fatigueBar.Render(engine->GetRenderTarget(), true);
}
