#include "LevelGUICommon.hpp"
#include "AdventureGroup.hpp"
#include "GameStatus.hpp"


void LevelGUICommon::Init(CGameEngine* _engine, GameStatus *_gameStatus, AdventureGroup* _adventureGroup)
{
	engine = _engine;
	gameStatus = _gameStatus;

	fatigueBar.Load(g_pTextures->fatigueBar, g_pTextures->fatigueBarFrame, _adventureGroup->GetFatiguePtr(), _adventureGroup->GetMaxFatiguePtr());
	fatigueBar.SetSmoothTransformationTime(0.7);
	fatigueBar.SetOffsetForInnerPart(6, 15);
	fatigueBar.SetPos(engine->GetWindowSize().x / 2 - fatigueBar.GetRect().width / 2, 3);

	resourcesStatusBar.Init(engine);
}

void LevelGUICommon::Update()
{
	resourcesStatusBar.Update(gameStatus->GetCardsAmount(), gameStatus->GetDiceAmount());
	fatigueBar.Update(g_pTimer->GetElapsedTime().asSeconds());
}

void LevelGUICommon::Render()
{
	resourcesStatusBar.Render();
	fatigueBar.Render(engine->GetRenderTarget());
}
