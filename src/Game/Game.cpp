#include "Game.hpp"
#include "LevelBuilder.hpp"
#include "ItemFactory.hpp"
#include "BattleGUI.hpp"
#include "LevelGUI.hpp"


Game::Game(CGameEngine* _engine, LevelType _levelType)
	: GameState(_engine)
	, levelType(_levelType)
	, adventureGroup(_engine, &notificationRenderer)
	, currentBattle(nullptr)
	, inBattle(false)
	, isPlayingBattleIntro(false)
	, afterIntroWaitingTime(2.0)
	, levelFinished(false)
	, speechBubbleManager(m_pGameEngine, &adventureGroup)
{
	view.reset(sf::FloatRect(0.0f, 0.0f, (float)_engine->GetWindowSize().x, (float)_engine->GetWindowSize().y));
	m_pGameEngine->GetRenderTarget().setView(view);

	level = LevelBuilder::buildLevel(levelType, g_pGameStatus->levels[levelType], &dialogManager);

	InitLevelGUI();

	blurShader.loadFromFile("Data/Shader/blur.frag", sf::Shader::Fragment);
	blurShader.setUniform("u_ImageSize", sf::Glsl::Vec2(m_pGameEngine->GetWindowSize().x, m_pGameEngine->GetWindowSize().y));
	blurShader.setUniform("u_texture", sf::Shader::CurrentTexture);
	blurShader.setUniform("u_kernelSize", 5.0f);
	blurShader.setUniform("u_blurringRect", sf::Glsl::Vec4(0.0f, 243.0f, m_pGameEngine->GetWindowSize().x, 785.0f));

	g_pMusic->SetCurrentEnvironment(MusicEnvironment(levelType + 1));
	g_pMusic->PlayMusic();
}

void Game::InitLevelGUI()
{
	LevelGUI *newGui = new LevelGUI(m_pGameEngine, &level->GetLevelStatus(), &adventureGroup, levelType, &notificationRenderer);
	newGui->SetOnLevelExitedCallback([&]() {HandleFinishedLevel(); });
	currentGUI = newGui;
}


Game::~Game()
{
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
	if (m_pGameEngine->GetKeystates(KeyID::Escape) == Keystates::Pressed)
	{
		if (inBattle)
		{
			SAFE_DELETE(currentBattle);
			inBattle = false;
		}

		g_pMusic->SetCurrentEnvironment(MusicEnvironment::mainRoomEnvironment);
		m_pGameEngine->GetRenderTarget().setView(m_pGameEngine->GetRenderTarget().getDefaultView());
		m_pGameEngine->PopState();
		m_pGameEngine->PopState();
		return;
	}

	m_pGameEngine->GetRenderTarget().setView(view);

	dialogManager.Update();
	speechBubbleManager.Update();
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

	HandlePlayerFatigue();

	g_pVideos->Update();
}


void Game::HandlePlayerFatigue()
{
	if (level->GetLevelStatus().GetRelativeFatigue() <= 0.33)
		adventureGroup.SetFatigueLevelOfAllPlayers(CombatantStatus::FatigueLevel::awake);
	else if (level->GetLevelStatus().GetRelativeFatigue() <= 0.66)
		adventureGroup.SetFatigueLevelOfAllPlayers(CombatantStatus::FatigueLevel::tired);
	else
		adventureGroup.SetFatigueLevelOfAllPlayers(CombatantStatus::FatigueLevel::stupid);
}


void Game::HandleBattleIntro()
{
	if (g_pSpritePool->newBattleAnimation->animationIsPlaying() == false)
	{
		if (afterIntroWaitingTime > 0.0)
			afterIntroWaitingTime -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
		else
		{
			isPlayingBattleIntro = false;
			g_pMusic->SetBattleStarted();

			if (currentBattle->isBossBattle && currentBattle->GetEnemies()[3] != nullptr)
			{
				if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::Greg)
					g_pVideos->PlayVideo(videoId::introGreg);
				else if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::Apachekid)
					g_pVideos->PlayVideo(videoId::introApacheKid);
				else if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::BigSpencer)
					g_pVideos->PlayVideo(videoId::introBigSpencer);
				else if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::TequilaJoe)
					g_pVideos->PlayVideo(videoId::introTequilaJoe);
				else if (currentBattle->GetEnemies()[3]->GetID() == CombatantID::BillNoface)
					g_pVideos->PlayVideo(videoId::introBillNoface);
			}
		}
	}
}



void Game::HandleFinishedLevel()
{
	if (!adventureGroup.IsDead())
	{
		g_pGameStatus->levels[levelType]++;
	}

	for (auto& [cards, dice, description] : levelRewards.achievements)
	{
		g_pGameStatus->AddCards(cards);
		g_pGameStatus->AddDice(dice);
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
			GetLevelRewards(levelRewards, level->GetLevelStatus(), false);
			dynamic_cast<LevelGUI*>(currentGUI)->OpenLevelFailedPanel(levelRewards);
		}
		else if (currentBattle->isBossBattle)
		{
			levelFinished = true;
			GetLevelRewards(levelRewards, level->GetLevelStatus(), true);
			dynamic_cast<LevelGUI*>(currentGUI)->OpenLevelFinishedPanel(levelRewards);
		}
		else
			OpenBattleRewardDialog();

		inBattle = false;
		SAFE_DELETE(currentBattle);

		g_pMusic->SetBattleEnded();
		speechBubbleManager.ActivateSpeechBubbles();
	}
}


void Game::OpenBattleRewardDialog()
{
	LootableDialog *dialog = new LootableDialog(m_pGameEngine, 0);
	dialog->SetTitle(std::string("Kampf gewonnen!"));

	for (int i = 0; i < 9; i++)
	{
		dialog->AddItem(ItemFactory::CreateBattleRewardItem(g_pGameStatus->levels[levelType], currentBattle->GetEnemies()));

		if (rand() % 5 != 0)
			break;
	}

	dialogManager.AddLootableDialog(dialog);
}


void Game::InitNewBattle()
{
	SAFE_DELETE(currentGUI);
	currentGUI = new BattleGUI(m_pGameEngine, &level->GetLevelStatus(), &notificationRenderer);

	speechBubbleManager.DeactivateSpeechBubbles();

	currentBattle = new Battle(view.getCenter().x, &adventureGroup, (BattleGUI*)currentGUI, m_pGameEngine, &notificationRenderer, level->GetEnemyIDs(), level->IsBossBattle(), &level->GetLevelStatus());

	isPlayingBattleIntro = true;
	afterIntroWaitingTime = 1.0;
	g_pSpritePool->newBattleAnimation->setCurrentAnimation("new_battle");
	g_pSpritePool->newBattleAnimation->setCurrentTime(0);
	g_pSpritePool->newBattleAnimation->reprocessCurrentTime();
	g_pSpritePool->newBattleAnimation->setPosition(SpriterEngine::point(view.getCenter().x - 300, m_pGameEngine->GetWindowSize().y / 2 - 30));
}



void Game::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);
	m_pGameEngine->ClearRenderTarget(sf::Color::Black);

	if (g_pVideos->IsPlayingVideo())
	{
		g_pVideos->Render(m_pGameEngine->GetWindow());
	}
	else 
	{
		m_pGameEngine->GetRenderTarget().setView(view);
		level->Render(m_pGameEngine->GetRenderTarget(), view.getCenter().x - view.getSize().x / 2);

		if (currentBattle == nullptr)
		{
			adventureGroup.Render();
			speechBubbleManager.Render();
		}
		else
		{
			currentBattle->Render();

			if (isPlayingBattleIntro)
			{
				g_pSpritePool->newBattleAnimation->setTimeElapsed(g_pTimer->GetElapsedTimeAsMilliseconds());
				g_pSpritePool->newBattleAnimation->render();
				g_pSpritePool->newBattleAnimation->playSoundTriggers();
			}
		}

		m_pGameEngine->GetRenderTarget().setView(m_pGameEngine->GetRenderTarget().getDefaultView());

		if (currentBattle != nullptr && currentBattle->CurrentlyExecutingAbility())
		{
			blurShader.setUniform("u_horizontalPass", 0.0f);
			m_pGameEngine->ApplyShaderToRenderTarget(&blurShader);
			blurShader.setUniform("u_horizontalPass", 1.0f);
			m_pGameEngine->ApplyShaderToRenderTarget(&blurShader);
			currentBattle->RenderAbilityAnimations();
		}

		currentGUI->Render();
		dialogManager.RenderDialogs();

		m_pGameEngine->GetRenderTarget().setView(view);
		notificationRenderer.Render(m_pGameEngine->GetRenderTarget());
		m_pGameEngine->FlushRenderTarget();
	}


	m_pGameEngine->FlipWindow();
}


void Game::SetOnGameFinishedCallback(std::function<void(void)> _onGameFinished)
{
	OnGameFinished = _onGameFinished;
}

