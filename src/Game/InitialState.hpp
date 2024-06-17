#pragma once

#include "../Framework/Gamestate.hpp"

#include "../Framework/Gui/Bar.hpp"
#include "../Framework/Graphics/Sprite.hpp"

#include "../Game/Resources/Resources.hpp"

#include <thread>
#include <atomic>


class CInitialState : public GameState
{
public:

	CInitialState(CGameEngine* _engine);
	~CInitialState();

	void Pause() override;
	void Resume() override;

	void HandleEvents() override;
	void Update() override;
	void Render(double _normalizedTimestep) override;

private:

	SpriterEngine::SpriterModel *modelLoadingScreen;
	SpriterEngine::EntityInstance *loadingScreen;

	std::atomic<bool> everythingLoaded;
	std::thread *loadingThread;
};

