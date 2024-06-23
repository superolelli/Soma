#include "InitialState.hpp"
#include "MainMenu.hpp"


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


CInitialState::CInitialState(CGameEngine *_engine)
	: GameState(_engine)
	, everythingLoaded(false)
{
	srand(time(0));

	_engine->UseSimpleRenderLoop(true);

	modelLoadingScreen = new SpriterEngine::SpriterModel("./Data/Sprites/LoadingScreen/loadingScreen.scml", new SpriterEngine::ExampleFileFactory(&_engine->GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine->GetRenderTarget()));
	loadingScreen = modelLoadingScreen->getNewEntityInstance("LoadingScreen");
	loadingScreen->setCurrentAnimation("loading");
	loadingScreen->setPlaybackSpeedRatio(0.65);
	loadingScreen->setPosition(SpriterEngine::point(0, 1080));
	loadingThread = new std::thread(&LoadAllData, &everythingLoaded, m_pGameEngine);
}


CInitialState::~CInitialState()
{
	SAFE_DELETE(modelLoadingScreen);
	SAFE_DELETE(loadingScreen);
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

		m_pGameEngine->UseSimpleRenderLoop(false);
		m_pGameEngine->ChangeStateImmediately(new MainMenu(m_pGameEngine));
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