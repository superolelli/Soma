#pragma once

#include "../engine/Gamestate.hpp"
#include "../engine/Graphics/Sprite.hpp"
#include "../game/Resources/Resources.hpp"
#include "../engine/Gameengine.hpp"
#include "../engine/Gui/Button.hpp"
#include "NameInputPanel.hpp"
#include "SavegamePanel.hpp"


class MainMenu : public GameState
{
public:

	MainMenu(CGameEngine* _engine);

	void Pause() override;
	void Resume() override;

	void HandleEvents() override;
	void Update() override;
	void Render(double _normalizedTimestep) override;

private:

	int activeGameslot;

	CSprite background;
	CButton buttonClose;
	SavegamePanel savegamePanels[3];
	NameInputPanel nameInputPanel;

	sf::Shader blurShader;
};