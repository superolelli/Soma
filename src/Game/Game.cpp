#include "Game.hpp"
#include "LevelBuilder.hpp"
#include "ItemFactory.hpp"


void Game::Init(CGameEngine * _engine)
{
	m_pGameEngine = _engine;

	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetWindow().setView(view);

	level = LevelBuilder::buildLevel(gameStatus->bangLevel, &dialogManager, gameStatus);
	adventureGroup.Init(_engine, &notificationRenderer, gameStatus);

	InitLevelGUI();

	currentBattle = nullptr;
	inBattle = false;
	isPlayingBattleIntro = false;
	afterIntroWaitingTime = 2.0f;
	levelFinished = false;

	g_pMusic->SetCurrentEnvironment(MusicEnvironment::bangEnvironment);
	g_pMusic->PlayMusic();
}


void Game::InitLevelGUI()
{
	LevelGUI *newGui = new LevelGUI;
	newGui->Init(m_pGameEngine, gameStatus, &adventureGroup);
	newGui->SetOnLevelExitedCallback([&]() {HandleFinishedLevel(); });
	currentGUI = newGui;
}


void Game::Cleanup()
{
	adventureGroup.Quit();
	dialogManager.Quit();
	m_pGameEngine = nullptr;
	SAFE_DELETE(currentGUI);
	SAFE_DELETE(level);
}


void Game::Pause()
{
}


void Game::Resume()
{
}


void Game::HandleEvents()
{
	m_pGameEngine->ProcessEvents();
}


void Game::Update()
{
	if(m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
		m_pGameEngine->StopEngine();

	m_pGameEngine->GetWindow().setView(view);

	dialogManager.Update();
	g_pMusic->Update();
	g_pSounds->Update();

	if (dialogManager.IsDialogOpen())
		return;

	if (!g_pVideos->IsPlayingVideo())
	{
		if (inBattle)
		{
			if (isPlayingBattleIntro)
			{
				adventureGroup.Update(0);
				HandleBattleIntro();
			}
			else
				UpdateBattle();
		}
		else
			UpdateLevel();

		notificationRenderer.Update();
		currentGUI->Update();
	}

	g_pVideos->Update();
}


void Game::HandleBattleIntro()
{
	if (g_pSpritePool->newBattleAnimation->animationIsPlaying() == false)
	{
		if (afterIntroWaitingTime > 0.0f)
			afterIntroWaitingTime -= g_pTimer->GetElapsedTime().asSeconds();
		else
		{
			isPlayingBattleIntro = false;
			g_pMusic->SetBattleStarted();

			if (currentBattle->isBossBattle)
			{
				if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::Greg)
					g_pVideos->PlayVideo(videoId::introGreg);
				else if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::Apachekid)
					g_pVideos->PlayVideo(videoId::introApacheKid);
				else if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::BigSpencer)
					g_pVideos->PlayVideo(videoId::introBigSpencer);
				else if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::TequilaJoe)
					g_pVideos->PlayVideo(videoId::introTequilaJoe);
			}
		}
	}
}



void Game::HandleFinishedLevel()
{
	if (!adventureGroup.IsDead())
	{
		gameStatus->AddDice(level->GetReward().dice);
		gameStatus->AddCards(level->GetReward().cards);
		gameStatus->bangLevel++;
	}
	OnGameFinished();
	g_pMusic->SetCurrentEnvironment(MusicEnvironment::mainRoomEnvironment);
	m_pGameEngine->PopState();
}


void Game::UpdateLevel()
{
	int xMove = 0;

	if (m_pGameEngine->GetKeystates(KeyID::Left) == Keystates::Held)
		xMove = -2;

	if (m_pGameEngine->GetKeystates(KeyID::Right) == Keystates::Held)
		xMove = 2;


	if (level->IsAtEnd(view.getCenter().x) || view.getCenter().x - view.getSize().x / 2 + xMove <= 0)
		xMove = 0;

	view.move(xMove, 0);

	level->Update(view.getCenter().x, m_pGameEngine);

	if (level->InBattle())
	{
		inBattle = true;
		InitNewBattle();
	}

	adventureGroup.Update(xMove);
}


void Game::UpdateBattle()
{
	adventureGroup.Update(0);
	currentBattle->Update();

	if (currentBattle->isFinished())
	{
		SAFE_DELETE(currentGUI);
		InitLevelGUI();

		if (adventureGroup.IsDead())
		{
			levelFinished = true;
			dynamic_cast<LevelGUI*>(currentGUI)->OpenLevelFailedPanel();
		}
		else if (currentBattle->isBossBattle)
		{
			levelFinished = true;
			dynamic_cast<LevelGUI*>(currentGUI)->OpenLevelFinishedPanel(level->GetReward());
		}
		else
			OpenBattleRewardDialog();

		inBattle = false;
		currentBattle->Quit();
		SAFE_DELETE(currentBattle);

		g_pMusic->SetBattleEnded();
	}
}


void Game::OpenBattleRewardDialog()
{
	LootableDialog *dialog = new LootableDialog;
	dialog->Init(m_pGameEngine, gameStatus, 0);
	dialog->SetTitle(std::string("Kampf gewonnen!"));

	for (int i = 0; i < 9; i++)
	{
		dialog->AddItem(ItemFactory::CreateBattleRewardItem(gameStatus->bangLevel, currentBattle->GetEnemies()));

		if (rand() % 5 != 0)
			break;
	}

	dialogManager.AddLootableDialog(dialog);
}


void Game::InitNewBattle()
{
	SAFE_DELETE(currentGUI);
	BattleGUI *newGui = new BattleGUI;
	newGui->Init(m_pGameEngine);
	newGui->SetAdventureGroup(&adventureGroup);
	currentGUI = newGui;

	currentBattle = new Battle;
	currentBattle->Init(view.getCenter().x, &adventureGroup, (BattleGUI*)currentGUI, m_pGameEngine, &notificationRenderer, level->GetEnemyIDs(), level->IsBossBattle());

	isPlayingBattleIntro = true;
	afterIntroWaitingTime = 2.0f;
	g_pSpritePool->newBattleAnimation->setCurrentAnimation("new_battle");
	g_pSpritePool->newBattleAnimation->setCurrentTime(0);
	g_pSpritePool->newBattleAnimation->reprocessCurrentTime();
	g_pSpritePool->newBattleAnimation->setPosition(SpriterEngine::point(view.getCenter().x - 300, m_pGameEngine->GetWindowSize().y / 2 - 30));
}



void Game::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);

	if (g_pVideos->IsPlayingVideo())
	{
		m_pGameEngine->GetWindow().setView(m_pGameEngine->GetWindow().getDefaultView());
		g_pVideos->Render(m_pGameEngine->GetWindow());
	}
	else 
	{
		m_pGameEngine->GetWindow().setView(view);
		level->Render(m_pGameEngine->GetWindow(), view.getCenter().x - view.getSize().x / 2);

		if (currentBattle == nullptr)
			adventureGroup.Render();
		else
		{
			currentBattle->Render();

			if (isPlayingBattleIntro)
			{
				g_pSpritePool->newBattleAnimation->setTimeElapsed(g_pTimer->GetElapsedTime().asMilliseconds());
				g_pSpritePool->newBattleAnimation->render();
				g_pSpritePool->newBattleAnimation->playSoundTriggers();
			}
		}

		notificationRenderer.Render(m_pGameEngine->GetWindow());

		m_pGameEngine->GetWindow().setView(m_pGameEngine->GetWindow().getDefaultView());

		if (currentBattle != nullptr)
			currentBattle->RenderAbilityAnimations();

		currentGUI->Render();
		dialogManager.RenderDialogs();
	}

	m_pGameEngine->FlipWindow();
}


void Game::SetOnGameFinishedCallback(std::function<void(void)> _onGameFinished)
{
	OnGameFinished = _onGameFinished;
}

