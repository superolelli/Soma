#include "VideoManager.hpp"
#include "MusicManager.hpp"

void VideoManager::LoadVideos(CGameEngine *_engine)
{
	isPlaying = false;
	engine = _engine;
	videos[videoId::introGreg].openFromFile("Data/Videos/Greg.mp4");
	videos[videoId::introApacheKid].openFromFile("Data/Videos/ApacheKid.mp4");
	videos[videoId::introBigSpencer].openFromFile("Data/Videos/BigSpencer.mp4");
}

void VideoManager::Update()
{
	if (videos[currentVideo].getStatus() == sfe::Status::Playing)
		videos[currentVideo].update();
	else if (isPlaying == true)
	{
		g_pMusic->PlayMusic();
		isPlaying = false;
		engine->UseSimpleRenderLoop(false);
		engine->GetWindow().setVerticalSyncEnabled(true);
	}
}

void VideoManager::Render(sf::RenderTarget &_target)
{
	if(videos[currentVideo].getStatus() == sfe::Status::Playing)
		_target.draw(videos[currentVideo]);
}

void VideoManager::PlayVideo(videoId _id)
{
	g_pMusic->StopMusic();
	isPlaying = true;
	engine->UseSimpleRenderLoop(true);
	engine->GetWindow().setVerticalSyncEnabled(false);
	currentVideo = _id;
	videos[currentVideo].play();
}

bool VideoManager::IsPlayingVideo()
{
	return videos[currentVideo].getStatus() == sfe::Status::Playing;
}
