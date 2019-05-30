#include "InitialState.hpp"



void CInitialState::Init(CGameEngine *_engine)
{
	m_pGameEngine = _engine;
}


void CInitialState::Cleanup()
{
	m_pGameEngine = nullptr;
}



void CInitialState::Pause()
{
}



void CInitialState::Resume()
{
}




void CInitialState::HandleEvents()
{
	m_pGameEngine->ProcessEvents();
}




void CInitialState::Update()
{
	//Load the resources
	g_pTextures->LoadTextures();
	g_pModels->LoadModels(*m_pGameEngine);
	g_pSpritePool->LoadSprites();
	g_pFonts->LoadFonts();
	g_pSounds->LoadSounds();
	g_pStringContainer->LoadStrings();
	g_pObjectProperties->LoadObjectProperties();
	g_pVideos->LoadVideos();

	m_pGameEngine->ChangeStateImmediately(new Game);
}



void CInitialState::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);

	//Renders everything

	m_pGameEngine->FlipWindow();
}

