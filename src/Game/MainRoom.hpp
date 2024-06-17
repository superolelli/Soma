#pragma once

#include "../Framework/Gamestate.hpp"
#include "../Game/Resources/Resources.hpp"

#include "ExitDialog.hpp"
#include "MainRoomGui.hpp"


class MainRoom : public GameState
{
public:

	MainRoom(CGameEngine* _engine);
	~MainRoom();

	void Pause() override;
	void Resume() override;

	void HandleEvents() override;
	void Update() override;
	void Render(double _normalizedTimestep) override;

private:

	CSprite background[4];
	SpriterEngine::EntityInstance *players[4];
	CSprite doors[3];
	SpriteWithText signs[3];
	CSprite roots;
	CSprite vendingMachine;

	sf::IntRect playerHitbox[4];

	MainRoomGUI gui;
	ExitDialog exitDialog;

	sf::View view;

	float xMovement;

	void InitDoors();
	void InitPlayers();

	void CheckForMovement();
	void HandlePlayerAnimation();
	void HandleDoors();
	void UpdateLevelSigns();

	void UpdatePlayerHitboxes();
	void HandleGUI();
	void CheckForClickedPlayer();
	void CheckForClickedVendingMachine();

	void RenderMainRoom();
};