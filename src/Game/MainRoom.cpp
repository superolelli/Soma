#include "MainRoom.hpp"


void MainRoom::Init(CGameEngine * _engine)
{
	m_pGameEngine = _engine;
	srand(time(0));

	int xPos = 0;
	for (int i = 0; i < 4; i++) 
	{
		background[i].Load(g_pTextures->mainRoomBackgrounds[i]);
		background[i].SetPos(xPos, 0);
		xPos += background[i].GetGlobalRect().width;
	}

	for (int i = 0; i < 3; i++)
	{
		doors[i].Load(g_pTextures->mainRoomDoors[i]);
		doors[i].SetPos(DoorPosition[i].x, DoorPosition[i].y);
	}

	players[0] = g_pModels->modelSimonMainRoom->getNewEntityInstance("Simon");
	players[1] = g_pModels->modelOleMainRoom->getNewEntityInstance("Ole");
	players[2] = g_pModels->modelAnnaMainRoom->getNewEntityInstance("Anna");
	players[3] = g_pModels->modelMarkusMainRoom->getNewEntityInstance("Markus");

	for (int i = 0; i < 4; i++) {
		players[i]->setCurrentAnimation("idle");
		players[i]->setPosition(SpriterEngine::point(PlayerPosition[i].x, PlayerPosition[i].y));
		players[i]->setPlaybackSpeedRatio(0.5f);
	}

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	xMovement = 0.0f;
}


void MainRoom::Cleanup()
{
	g_pModels->Quit();  //Muss in letztem Gamestate passieren
	g_pSpritePool->FreeSprites();   //Muss in letztem Gamestate passieren
	m_pGameEngine = nullptr;

	for(auto p : players)
		SAFE_DELETE(p);
}


void MainRoom::Pause()
{
}


void MainRoom::Resume()
{
}


void MainRoom::HandleEvents()
{
	m_pGameEngine->ProcessEvents();
}


void MainRoom::Update()
{
	if (m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
		m_pGameEngine->StopEngine();

	if(m_pGameEngine->GetKeystates(KeyID::G) == Keystates::Pressed)
		m_pGameEngine->ChangeStateImmediately(new MainRoom);

	CheckForMovement();
	HandlePlayerAnimation();

	m_pGameEngine->GetWindow().setView(view);
}


void MainRoom::CheckForMovement() 
{
	xMovement = 0.0;

	if (m_pGameEngine->GetMousePos().x < 50) {
		xMovement = - (50.0 - static_cast<float>(m_pGameEngine->GetMousePos().x)) / 2.0;
	}
	else if (m_pGameEngine->GetMousePos().x > m_pGameEngine->GetWindowSize().x - 50) {
		xMovement = static_cast<float>(m_pGameEngine->GetMousePos().x - (m_pGameEngine->GetWindowSize().x - 50)) / 2.0;
	}
}


void MainRoom::HandlePlayerAnimation() 
{
	for (auto p : players) {
		if (p->animationJustFinished() && p->currentAnimationName() == "busy")
			p->setCurrentAnimation("idle");

		if (p->animationJustLooped())
		{
			if (rand() % 5 == 0)
				p->setCurrentAnimation("busy");
		}
	}
}


void MainRoom::Render(double _normalizedTimestep)
{
	if (view.getCenter().x - m_pGameEngine->GetWindowSize().x / 2 + xMovement * _normalizedTimestep >= 0
		&& view.getCenter().x + m_pGameEngine->GetWindowSize().x / 2 + xMovement * _normalizedTimestep < background[0].GetGlobalRect().width * 4) {
		view.move(xMovement * _normalizedTimestep, 0);
	}

	m_pGameEngine->ClearWindow(sf::Color::Black);

	m_pGameEngine->GetWindow().setView(view);

	sf::IntRect viewRect = {
		(int)(view.getCenter().x - view.getSize().x / 2),
		(int)(view.getCenter().y - view.getSize().y /2),
		(int)view.getSize().x,
		(int)view.getSize().y};

	for (auto &b : background) {
		if (b.GetGlobalRect().intersects(viewRect))
			b.Render(m_pGameEngine->GetWindow());
	}

	for (auto &d : doors) {
		if (d.GetGlobalRect().intersects(viewRect))
			d.Render(m_pGameEngine->GetWindow());
	}

	for (auto p : players) {
		p->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
		p->render();
		p->playSoundTriggers();
	}

	m_pGameEngine->GetWindow().setView(m_pGameEngine->GetWindow().getDefaultView());

	// GUI

	m_pGameEngine->FlipWindow();
}

