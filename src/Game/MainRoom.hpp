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

	sf::Vector2i DoorPosition[3] = { { 3992, 240 },{ 4651, 240 },{ 5299, 240 } };
	sf::Vector2i PlayerPosition[4] = {{ 2040, 770 },{ 177, 850 },{ 3040, 830 },{ 1006, 760 } };

	CSprite background[4];
	SpriterEngine::EntityInstance *players[4];
	CSprite doors[3];

	sf::View view;

	float xMovement;

	void CheckForMovement();
	void HandlePlayerAnimation();
};