#pragma once

#include "../../Framework/Patterns/Singleton.hpp"
#include <sfeMovie\Movie.hpp>
#include "../../Framework/Gameengine.hpp"

enum class videoId { introGreg };


#define g_pVideos VideoManager::Get()
class VideoManager : public TSingleton<VideoManager> {
public:

	void LoadVideos(CGameEngine *_engine);

	void Update();
	void Render(sf::RenderTarget &_target);

	void PlayVideo(videoId _id);
	bool IsPlayingVideo();

private:

	sfe::Movie introGreg;

	videoId currentVideo;
	CGameEngine *engine;

	bool isPlaying;
};