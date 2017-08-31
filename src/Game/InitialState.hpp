#pragma once

#include "../Framework/Gamestate.hpp"

#include "../Framework/Gui/Beam.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include "../Game/Resources/Resources.hpp"



class CInitialState : public GameState
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
};

