#include "MainMenu.hpp"
#include "MainRoom.hpp"
#include "SavegameManager.hpp"


void MainMenu::Init(CGameEngine* _engine)
{
	m_pGameEngine = _engine;

	activeGameslot = -1;

	background.Load(g_pTextures->mainMenuBackground);

	buttonClose.Load(g_pTextures->lootablePanelCloseButton, Buttontypes::Motion_Up);
	buttonClose.SetPos(1824, 19);

	for (auto& s : savegamePanels)
		s.Init(m_pGameEngine);

	savegamePanels[0].SetPos(50, 500);
	savegamePanels[0].SetName(SavegameManager::GetSavegameName(0));

	savegamePanels[1].SetPos(650, 500);
	savegamePanels[1].SetName(SavegameManager::GetSavegameName(1));

	savegamePanels[2].SetPos(1250, 500);
	savegamePanels[2].SetName(SavegameManager::GetSavegameName(2));

	nameInputPanel.Init("Neues Spiel");
	nameInputPanel.SetVisible(false);
	nameInputPanel.SetPos(_engine->GetWindowSize().x / 2 - nameInputPanel.GetRect().width / 2, _engine->GetWindowSize().y / 2 - nameInputPanel.GetRect().height / 2);

	blurShader.loadFromFile("Data/Shader/blur.frag", sf::Shader::Fragment);
	blurShader.setUniform("u_ImageSize", sf::Glsl::Vec2(m_pGameEngine->GetWindowSize().x, m_pGameEngine->GetWindowSize().y));
	blurShader.setUniform("u_texture", sf::Shader::CurrentTexture);
	blurShader.setUniform("u_kernelSize", 5.0f);
	blurShader.setUniform("u_blurringRect", sf::Glsl::Vec4(0.0f, 0.0f, m_pGameEngine->GetWindowSize().x, m_pGameEngine->GetWindowSize().y));
}

void MainMenu::Cleanup()
{
	m_pGameEngine = nullptr;
}

void MainMenu::Pause()
{
}

void MainMenu::Resume()
{
}

void MainMenu::HandleEvents()
{
	m_pGameEngine->ProcessEvents();
}


void MainMenu::Update()
{
	if (buttonClose.Update(*m_pGameEngine) == true)
	{
		m_pGameEngine->StopEngine();
		g_pModels->Quit();  //Muss in letztem Gamestate passieren
		g_pSpritePool->FreeSprites();   //Muss in letztem Gamestate passieren
		g_pSounds->Quit(); //Muss in letztem Gamestate passieren
	}

	if (nameInputPanel.Update(m_pGameEngine))
	{
		nameInputPanel.SetVisible(false);
		savegamePanels[activeGameslot].SetName(nameInputPanel.GetString());

		auto gameStatus = new GameStatus();
		gameStatus->Init();
		gameStatus->SetFilepath("Data/Saves/" + std::to_string(activeGameslot) + "_" + nameInputPanel.GetString());
		SavegameManager::StoreSavegame(gameStatus);
		m_pGameEngine->ChangeStateImmediately(new MainRoom(gameStatus));
		return;
	}

	if (activeGameslot < 0) {
		for (int i = 0; i < 3; i++)
		{
			auto updateResult = savegamePanels[i].Update();
			if (updateResult == SavegamePanel::SavegameState::Continue) {
				auto gameStatus = SavegameManager::LoadSavegame(i);
				m_pGameEngine->ChangeStateImmediately(new MainRoom(gameStatus));
				return;
			}	
			else if (updateResult == SavegamePanel::SavegameState::Create) {
				activeGameslot = i;
				nameInputPanel.SetVisible(true);
			}
			else if(updateResult == SavegamePanel::SavegameState::Delete){
				SavegameManager::DeleteSavegame(i);
			}
		}
			
	}
}


void MainMenu::Render(double _normalizedTimestep)
{
	m_pGameEngine->ClearWindow(sf::Color::Black);
	m_pGameEngine->ClearRenderTarget(sf::Color::Black);

	background.Render(m_pGameEngine->GetRenderTarget());

	buttonClose.Render(m_pGameEngine->GetRenderTarget());

	for (auto &s : savegamePanels)
		s.Render();

	if (activeGameslot > -1) {
		blurShader.setUniform("u_horizontalPass", 0.0f);
		m_pGameEngine->ApplyShaderToRenderTarget(&blurShader);
		blurShader.setUniform("u_horizontalPass", 1.0f);
		m_pGameEngine->ApplyShaderToRenderTarget(&blurShader);
	}

	nameInputPanel.Render(m_pGameEngine->GetRenderTarget());

	m_pGameEngine->FlushRenderTarget();
	m_pGameEngine->FlipWindow();
}