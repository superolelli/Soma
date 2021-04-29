#pragma once

#include "../Framework/Gamestate.hpp"
#include "../Framework/Graphics/Sprite.hpp"
#include "../Game/Resources/Resources.hpp"
#include "../Framework/Gameengine.hpp"
#include "../Framework/Gui/Button.hpp"
#include "NameInputPanel.hpp"
#include "SavegamePanel.hpp"


class MainMenu : public GameState
{
public:

	void Init(CGameEngine* _engine) override;
	void Cleanup() override;

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