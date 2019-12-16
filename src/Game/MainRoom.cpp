#include "MainRoom.hpp"
#include "Game.hpp"


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
		doors[i].SetPos(g_pObjectProperties->mainRoomDoorPositions[i].x, g_pObjectProperties->mainRoomDoorPositions[i].y);

		signs[i].Load(g_pTextures->mainRoomSigns[i]);
		signs[i].SetPos(g_pObjectProperties->mainRoomSignPositions[i].x, g_pObjectProperties->mainRoomSignPositions[i].y);
		auto textID = signs[i].AddText("1");
	}

	signs[0].SetTextFont(0, g_pFonts->f_tiza);
	signs[0].SetTextCharacterSize(0, 70);
	signs[0].SetTextPosCentered(0);
	signs[0].MoveText(0, 0, -20);

	signs[1].SetTextFont(0, g_pFonts->f_blackwoodCastle);
	signs[1].SetTextCharacterSize(0, 70);
	signs[1].SetTextPosCentered(0);
	signs[1].MoveText(0, 0, -45);

	signs[2].SetTextFont(0, g_pFonts->f_shanghai);
	signs[2].SetTextCharacterSize(0, 70);
	signs[2].SetTextPosCentered(0);
	signs[2].MoveText(0, -55, -40);

	players[0] = g_pModels->modelSimonMainRoom->getNewEntityInstance("Simon");
	players[1] = g_pModels->modelOleMainRoom->getNewEntityInstance("Ole");
	players[2] = g_pModels->modelAnnaMainRoom->getNewEntityInstance("Anna");
	players[3] = g_pModels->modelMarkusMainRoom->getNewEntityInstance("Markus");

	for (int i = 0; i < 4; i++) {
		players[i]->setCurrentAnimation("idle");
		players[i]->setPosition(SpriterEngine::point(g_pObjectProperties->mainRoomPlayerPositions[i].x, g_pObjectProperties->mainRoomPlayerPositions[i].y));
		players[i]->setPlaybackSpeedRatio(0.5f);
		players[i]->reprocessCurrentTime();
	}

	UpdatePlayerHitboxes();

	for (int i = 0; i < 4; i++)
	{
		players[i]->enableSpatialSounds();
		players[i]->setSoundAttenuation(5);
		players[i]->setSoundMinDistance(900);
		players[i]->setSoundPosition(playerHitbox[i].left + playerHitbox[i].width / 2, playerHitbox[i].top + playerHitbox[i].height / 2);
	}

	roots.Load(g_pTextures->mainRoomRoots);
	roots.SetPos(g_pObjectProperties->mainRoomRootsPosition.x, g_pObjectProperties->mainRoomRootsPosition.y);

	gameStatus.Init(m_pGameEngine);

	skillPanel.Init(&gameStatus, m_pGameEngine);
	inventory.Init(&gameStatus, m_pGameEngine);
	consumablePanel.Init(m_pGameEngine, &gameStatus);

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	xMovement = 0.0f;
}


void MainRoom::UpdatePlayerHitboxes()
{
	for (int i = 0; i < 4; i++)
	{
		SpriterEngine::UniversalObjectInterface* hitboxObj;
		hitboxObj = players[i]->objectIfExistsOnCurrentFrame("hitbox");
		
		playerHitbox[i].left = hitboxObj->getPosition().x;
		playerHitbox[i].top = hitboxObj->getPosition().y;
		playerHitbox[i].width = hitboxObj->getSize().x  * hitboxObj->getScale().x;
		playerHitbox[i].height = hitboxObj->getSize().y * hitboxObj->getScale().y;
	}
}

void MainRoom::Cleanup()
{
	m_pGameEngine = nullptr;

	for(auto p : players)
		SAFE_DELETE(p);

	inventory.Quit();
	consumablePanel.Quit();

	g_pModels->Quit();  //Muss in letztem Gamestate passieren
	g_pSpritePool->FreeSprites();   //Muss in letztem Gamestate passieren
	g_pSounds->Quit(); //Muss in letztem Gamestate passieren
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
	UpdateLevelSigns();
	skillPanel.Update();
	inventory.Update();
	consumablePanel.Update();

	m_pGameEngine->GetWindow().setView(view);

	if (m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
		m_pGameEngine->StopEngine();

	if (m_pGameEngine->GetKeystates(KeyID::I) == Keystates::Released && !inventory.IsOpen())
		inventory.Open();

	//if (m_pGameEngine->GetKeystates(KeyID::O) == Keystates::Released)
	//{
	//	Item newItem;
	//	newItem.color = sf::Color(128, 128, 0);
	//	newItem.id = ItemID(rand() % ItemID::numberOfItems);
	//	gameStatus.AddItem(newItem);
	//}

	if (!inventory.IsOpen())
	{
		for (int i = 0; i < 4; i++)
		{
			if (playerHitbox[i].contains(m_pGameEngine->GetWorldMousePos()))
			{
				m_pGameEngine->SetCursor(sf::Cursor::Type::Hand);

				if (m_pGameEngine->GetButtonstates(ButtonID::Left) == Released && !skillPanel.IsOpen())
					skillPanel.Open(i);
			}
		}
	}

	CheckForMovement();
	HandlePlayerAnimation();
	UpdatePlayerHitboxes();
	HandleDoors();
	g_pSounds->Update();
}



void MainRoom::UpdateLevelSigns()
{
	signs[0].ChangeString(0, std::to_string(gameStatus.bangLevel));
	signs[1].ChangeString(0, std::to_string(gameStatus.kutschfahrtLevel));
	signs[2].ChangeString(0, std::to_string(gameStatus.tichuLevel));
}


void MainRoom::CheckForMovement() 
{
	xMovement = 0.0;

	if (m_pGameEngine->GetMousePos().x < 100) {
		xMovement = - (100.0 - static_cast<float>(m_pGameEngine->GetMousePos().x));
	}
	else if (m_pGameEngine->GetMousePos().x > m_pGameEngine->GetWindowSize().x - 100) {
		xMovement = static_cast<float>(m_pGameEngine->GetMousePos().x - (m_pGameEngine->GetWindowSize().x - 100));
	}
}


void MainRoom::HandlePlayerAnimation() 
{
	for (auto p : players) {
		if (p->animationJustFinished() && p->currentAnimationName() == "busy")
			p->setCurrentAnimation("idle");

		if (p->animationJustLooped())
		{
			if (rand() % 6 == 0) {
				p->setCurrentAnimation("busy");
				p->setCurrentTime(0);
			}
		}
	}
}


void MainRoom::HandleDoors()
{
	for (auto &d : doors)
	{
		if (d.GetGlobalRect().contains(m_pGameEngine->GetWorldMousePos()) && !skillPanel.IsOpen() && !inventory.IsOpen()) 
		{
			m_pGameEngine->SetCursor(sf::Cursor::Type::Hand);

			if (m_pGameEngine->GetButtonstates(ButtonID::Left) == Released)
			{
				g_pSounds->PlaySound(soundID::DOOR);
				auto newGame = new Game();
				newGame->SetGameStatusPtr(&gameStatus);
				newGame->SetConsumablePanelPtr(&consumablePanel);
				m_pGameEngine->PushState(newGame);
			}
		}
	}
}


void MainRoom::Render(double _normalizedTimestep)
{
	if (view.getCenter().x - m_pGameEngine->GetWindowSize().x / 2 + xMovement * _normalizedTimestep >= 0
		&& view.getCenter().x + m_pGameEngine->GetWindowSize().x / 2 + xMovement * _normalizedTimestep < background[0].GetGlobalRect().width * 4) {
		view.move(xMovement * _normalizedTimestep, 0);
		sf::Listener::setPosition(sf::Vector3f(view.getCenter().x, view.getCenter().y, -5));
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

	if (roots.GetGlobalRect().intersects(viewRect))
		roots.Render(m_pGameEngine->GetWindow());

	for (auto &s : signs) {
		if (s.GetRect().intersects(viewRect))
			s.Render(m_pGameEngine->GetWindow());
	}

	for (auto p : players) {
		p->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
		p->render();
		p->playSoundTriggers();
	}

	m_pGameEngine->GetWindow().setView(m_pGameEngine->GetWindow().getDefaultView());

	// GUI
	gameStatus.RenderStatusBar();
	skillPanel.Render();
	inventory.Render();
	consumablePanel.Render();

	m_pGameEngine->FlipWindow();
}

