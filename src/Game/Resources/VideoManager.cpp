#include "VideoManager.hpp"


void VideoManager::LoadVideos()
{
	introGreg.openFromFile("Data/Videos/Greg.mp4");
}

void VideoManager::Update()
{
	if(introGreg.getStatus() == sfe::Status::Playing)
		introGreg.update();
}

void VideoManager::Render(sf::RenderTarget &_target)
{
	if(introGreg.getStatus() == sfe::Status::Playing)
		_target.draw(introGreg);
}

void VideoManager::PlayVideo(videoId _id)
{
	if (_id == videoId::introGreg)
		introGreg.play();
}

bool VideoManager::IsPlayingVideo()
{
	return introGreg.getStatus() == sfe::Status::Playing;
}
