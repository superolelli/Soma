#include "MainRoom.hpp"
#include "Game.hpp"
#include "ItemFactory.hpp"


MainRoom::MainRoom(CGameEngine* _engine)
	: GameState (_engine)
	, gui(_engine)
	, exitDialog(_engine, "Speichern und ins Hauptmen�?", 30)
	, background{ {g_pTextures->mainRoomBackgrounds[0]},
		{g_pTextures->mainRoomBackgrounds[1]},
		{g_pTextures->mainRoomBackgrounds[2]},
		{g_pTextures->mainRoomBackgrounds[3]} }
	, vendingMachine(g_pTextures->mainRoomVendingMachine)
	, doors{ {g_pTextures->mainRoomDoors[0]}, {g_pTextures->mainRoomDoors[1]}, {g_pTextures->mainRoomDoors[2]} }
	, signs{ {g_pTextures->mainRoomSigns[0]}, {g_pTextures->mainRoomSigns[1]}, {g_pTextures->mainRoomSigns[2]} }
	, roots(g_pTextures->mainRoomRoots)
	, xMovement(0.0f)
{
	int xPos = 0;
	for (int i = 0; i < 4; i++)
	{
		background[i].SetPos(xPos, 0);
		xPos += background[i].GetGlobalRect().width;
	}

	InitDoors();
	InitPlayers();

	vendingMachine.SetPos(g_pObjectProperties->mainRoomVendingMachinePosition.x, g_pObjectProperties->mainRoomVendingMachinePosition.y);

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetRenderTarget().setView(view);
}


MainRoom::~MainRoom()
{
	for (auto p : players)
		SAFE_DELETE(p);
}


void MainRoom::InitDoors()
{
	for (int i = 0; i < 3; i++)
	{
		doors[i].SetPos(g_pObjectProperties->mainRoomDoorPositions[i].x, g_pObjectProperties->mainRoomDoorPositions[i].y);
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

	roots.SetPos(g_pObjectProperties->mainRoomRootsPosition.x, g_pObjectProperties->mainRoomRootsPosition.y);
}


void MainRoom::InitPlayers()
{
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

	m_pGameEngine->GetRenderTarget().setView(view);

	if (m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
		exitDialog.SetOpen(true);

	if (m_pGameEngine->GetKeystates(KeyID::F5) == Keystates::Released)
		g_pGameStatus->StoreToFile();

	g_pSounds->Update();
	HandleGUI();

	if (!exitDialog.IsOpen()) {
		CheckForMovement();
		HandlePlayerAnimation();
		UpdatePlayerHitboxes();
		HandleDoors();

		if (view.getCenter().x - m_pGameEngine->GetWindowSize().x / 2 + xMovement >= 0
			&& view.getCenter().x + m_pGameEngine->GetWindowSize().x / 2 + xMovement < background[0].GetGlobalRect().width * 4) {
			view.move(xMovement, 0);
			sf::Listener::setPosition(sf::Vector3f(view.getCenter().x, view.getCenter().y, -5));
		}
	}
}



void MainRoom::HandleGUI()
{
	if (!gui.IsPanelOpen() && !exitDialog.IsOpen())
	{
		CheckForClickedPlayer();
		CheckForClickedVendingMachine();
	}

	m_pGameEngine->GetRenderTarget().setView(m_pGameEngine->GetRenderTarget().getDefaultView());

	exitDialog.Update();
	if (exitDialog.YesChosen())
	{
		g_pGameStatus->StoreToFile();
		m_pGameEngine->GetRenderTarget().setView(m_pGameEngine->GetRenderTarget().getDefaultView());
		m_pGameEngine->PopState();
		return;
	}

	gui.Update();
	m_pGameEngine->GetRenderTarget().setView(view);
}


void MainRoom::CheckForClickedPlayer()
{
	for (int i = 0; i < 4; i++)
	{
		if (playerHitbox[i].contains(m_pGameEngine->GetWorldMousePos()))
		{
			m_pGameEngine->SetCursor(sf::Cursor::Type::Hand);

			if (m_pGameEngine->GetButtonstates(ButtonID::Left) == Released)
				gui.PlayerClicked(i);
		}
	}
}

void MainRoom::CheckForClickedVendingMachine()
{
	if (vendingMachine.GetGlobalRect().contains(m_pGameEngine->GetWorldMousePos()))
	{
		m_pGameEngine->SetCursor(sf::Cursor::Type::Hand);
		if (m_pGameEngine->GetButtonstates(ButtonID::Left) == Released)
			gui.VendingMachineClicked();
	}
}


void MainRoom::UpdateLevelSigns()
{
	signs[0].ChangeString(0, std::to_string(g_pGameStatus->levels[LevelType::bang]));
	signs[1].ChangeString(0, std::to_string(g_pGameStatus->levels[LevelType::kutschfahrt]));
	signs[2].ChangeString(0, std::to_string(g_pGameStatus->levels[LevelType::tichu]));
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
	for (int i = 0; i < 3; i++)
	{
		if (doors[i].GetGlobalRect().contains(m_pGameEngine->GetWorldMousePos()) && !gui.IsPanelOpen()) 
		{
			m_pGameEngine->SetCursor(sf::Cursor::Type::Hand);

			if (m_pGameEngine->GetButtonstates(ButtonID::Left) == Released)
			{
				if (i == 0)
					g_pSounds->PlaySound(soundID::DOOR);
				else
					g_pSounds->PlaySound(soundID::DOOR_KUTSCHFAHRT);

				auto newGame = new Game(m_pGameEngine, static_cast<LevelType>(i));
				newGame->SetOnGameFinishedCallback([&]() {	g_pGameStatus->StoreToFile(); gui.ChooseNewShopItems(); });
				m_pGameEngine->PushState(newGame);
			}
		}
	}
}


void MainRoom::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearRenderTarget(sf::Color::Black);
	m_pGameEngine->GetRenderTarget().setView(view);

	RenderMainRoom();

	m_pGameEngine->GetRenderTarget().setView(m_pGameEngine->GetRenderTarget().getDefaultView());
	gui.Render();
	exitDialog.Render();

	m_pGameEngine->FlushRenderTarget();
	m_pGameEngine->FlipWindow();
}


void MainRoom::RenderMainRoom()
{
	sf::IntRect viewRect = {
		(int)(view.getCenter().x - view.getSize().x / 2),
		(int)(view.getCenter().y - view.getSize().y / 2),
		(int)view.getSize().x,
		(int)view.getSize().y };

	for (auto &b : background) {
		if (b.GetGlobalRect().intersects(viewRect))
			b.Render(m_pGameEngine->GetRenderTarget());
	}

	for (auto &d : doors) {
		if (d.GetGlobalRect().intersects(viewRect))
			d.Render(m_pGameEngine->GetRenderTarget());
	}

	if (roots.GetGlobalRect().intersects(viewRect))
		roots.Render(m_pGameEngine->GetRenderTarget());

	for (auto &s : signs) {
		if (s.GetRect().intersects(viewRect))
			s.Render(m_pGameEngine->GetRenderTarget());
	}

	for (auto p : players) {
		p->setTimeElapsed(g_pTimer->GetElapsedTimeAsMilliseconds());
		p->render();
		p->playSoundTriggers();
	}

	vendingMachine.Render(m_pGameEngine->GetRenderTarget());
}