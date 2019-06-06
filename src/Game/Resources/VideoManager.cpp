#include "VideoManager.hpp"


void VideoManager::LoadVideos(CGameEngine *_engine)
{
	isPlaying = false;
	engine = _engine;
	introGreg.openFromFile("Data/Videos/Greg.mp4");
}

void VideoManager::Update()
{
	if (introGreg.getStatus() == sfe::Status::Playing)
		introGreg.update();
	else if (isPlaying == true)
	{
		isPlaying = false;
		engine->UseSimpleRenderLoop(false);
		engine->GetWindow().setVerticalSyncEnabled(true);
	}
}

void VideoManager::Render(sf::RenderTarget &_target)
{
	if(introGreg.getStatus() == sfe::Status::Playing)
		_target.draw(introGreg);
}

void VideoManager::PlayVideo(videoId _id)
{
	if (_id == videoId::introGreg)
	{
		isPlaying = true;
		engine->UseSimpleRenderLoop(true);
		engine->GetWindow().setVerticalSyncEnabled(false);
		introGreg.play();
	}
}

bool VideoManager::IsPlayingVideo()
{
	return introGreg.getStatus() == sfe::Status::Playing;
}
