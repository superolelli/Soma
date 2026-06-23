#pragma once

#include "../../engine/Patterns/Singleton.hpp"
#include <sfeMovie\Movie.hpp>
#include "../../engine/Gameengine.hpp"

enum videoId { introGreg, introApacheKid, introBigSpencer, introTequilaJoe, introBillNoface };


#define g_pVideos VideoManager::Get()
class VideoManager : public TSingleton<VideoManager> {
public:

	void LoadVideos(CGameEngine *_engine);

	void Update();
	void Render(sf::RenderTarget &_target);

	void PlayVideo(videoId _id);
	bool IsPlayingVideo();

private:

	sfe::Movie videos[5];


	videoId currentVideo;
	CGameEngine *engine;

	bool isPlaying;
};