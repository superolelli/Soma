#include "InitialState.hpp"
#include "MainRoom.hpp"


void LoadAllData(std::atomic<bool> *_everythingLoaded, CGameEngine *_engine)
{
	g_pTextures->LoadTextures();
	g_pStringContainer->LoadStrings();
	g_pObjectProperties->LoadObjectProperties();
    g_pFonts->LoadFonts();
    g_pSounds->LoadSounds();
    g_pModels->LoadModels(*_engine);
    g_pSpritePool->LoadSprites();
	g_pVideos->LoadVideos(_engine);
	g_pMusic->LoadMusic();
	*_everythingLoaded = true;
}


void CInitialState::Init(CGameEngine *_engine)
{
	m_pGameEngine = _engine;

	_engine->UseSimpleRenderLoop(true);

	modelLoadingScreen = new SpriterEngine::SpriterModel("./Data/Sprites/LoadingScreen/loadingScreen.scml", new SpriterEngine::ExampleFileFactory(&_engine->GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine->GetRenderTarget()));
	loadingScreen = modelLoadingScreen->getNewEntityInstance("LoadingScreen");
	loadingScreen->setCurrentAnimation("loading");
	loadingScreen->setPlaybackSpeedRatio(0.65);
	loadingScreen->setPosition(SpriterEngine::point(0, 1080));
	everythingLoaded = false;
	loadingThread = new std::thread(&LoadAllData, &everythingLoaded, m_pGameEngine);
}


void CInitialState::Cleanup()
{
	m_pGameEngine->UseSimpleRenderLoop(false);
	SAFE_DELETE(modelLoadingScreen);
	SAFE_DELETE(loadingScreen);
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
	if (everythingLoaded)
	{
		loadingThread->join();

		SAFE_DELETE(loadingThread);
		SAFE_DELETE(modelLoadingScreen);
		SAFE_DELETE(loadingScreen);

		m_pGameEngine->ChangeStateImmediately(new MainRoom);
	}
}



void CInitialState::Render(double _normalizedTimestep)
{
	using namespace std::chrono_literals;

	m_pGameEngine->ClearWindow(sf::Color::Black);
	m_pGameEngine->ClearRenderTarget(sf::Color::Black);

	loadingScreen->setTimeElapsed(g_pTimer->GetElapsedTimeAsMilliseconds());
	loadingScreen->render();

	m_pGameEngine->FlushRenderTarget();
	m_pGameEngine->FlipWindow();

	std::this_thread::sleep_for(20ms);
}