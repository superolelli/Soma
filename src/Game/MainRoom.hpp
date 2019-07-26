#pragma once

#include "../Framework/Gamestate.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include "../Game/Resources/Resources.hpp"


class MainRoom : public GameState
{
public:

	void Init(CGameEngine *_engine) override;
	void Cleanup() override;

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

	sf::View view;

	float xMovement;

	void CheckForMovement();
	void HandlePlayerAnimation();
	void HandleDoors();
};