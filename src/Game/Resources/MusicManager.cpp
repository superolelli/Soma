#include "MusicManager.hpp"
#include "../../Framework/Time.hpp"

void MusicManager::LoadMusic()
{
	mainRoomMusic.openFromFile("Data/Music/MainRoom.ogg");
	mainRoomMusic.setVolume(20);
	mainRoomMusic.setRelativeToListener(true);
	mainRoomMusic.setLoop(true);

	bangBackgroundMusic[0].openFromFile("Data/Music/Bang1.ogg");
	bangBackgroundMusic[1].openFromFile("Data/Music/Bang2.ogg");
	bangBackgroundMusic[2].openFromFile("Data/Music/Bang3.ogg");
	bangBackgroundMusic[3].openFromFile("Data/Music/Bang4.ogg");
	bangBackgroundMusic[4].openFromFile("Data/Music/Bang5.ogg");
	bangBackgroundMusic[5].openFromFile("Data/Music/Bang6.ogg");
	bangBackgroundMusic[6].openFromFile("Data/Music/Bang7.ogg");

	for (auto &m : bangBackgroundMusic)
	{
		m.setRelativeToListener(true);
		m.setVolume(50);
	}

	bangBattleMusic.openFromFile("Data/Music/Bang_battle.ogg");
	bangBattleMusic.setRelativeToListener(true);
	bangBattleMusic.setLoop(true);
	bangBattleMusic.setVolume(50);

	currentEnvironment = MusicEnvironment::mainRoomEnvironment;
	currentMusic = &mainRoomMusic;
	currentBattleMusic = nullptr;

	blendInAfterBattleTime = 0.0;

	nextTracks = { 0, 1, 2, 3, 4, 5, 6 };

	currentMusic->play();
}


void MusicManager::Update()
{
	if (currentEnvironment == MusicEnvironment::mainRoomEnvironment)
		return;

	if (currentMusic->getStatus() == sf::SoundSource::Status::Stopped)
	{
		currentMusic = &bangBackgroundMusic[GetNextTrack()];
		currentMusic->play();
	}

	if (blendInAfterBattleTime > 0.0)
	{
		blendInAfterBattleTime -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
		blendInAfterBattleTime = std::max(blendInAfterBattleTime, 0.0);
		currentMusic->setVolume(50.0 * (4.0 - blendInAfterBattleTime) / 4.0);
		currentBattleMusic->setVolume(50.0 * (blendInAfterBattleTime / 4.0));

		if (blendInAfterBattleTime <= 0.0)
		{
			currentBattleMusic->stop();
			currentBattleMusic = nullptr;
		}
	}
}


int MusicManager::GetNextTrack()
{
	if (nextTracks.empty())
		nextTracks = {0, 1, 2, 3, 4, 5, 6};

	int trackPosition = rand() % nextTracks.size();

	int trackNumber = nextTracks[trackPosition];
	nextTracks.erase(nextTracks.begin() + trackPosition);

	return trackNumber;
}


void MusicManager::SetCurrentEnvironment(MusicEnvironment _environmentID)
{
	if (currentEnvironment == _environmentID)
		return;

	if(currentMusic)
		currentMusic->stop();

	if (currentBattleMusic)
		currentBattleMusic->stop();

	currentEnvironment = _environmentID;

	if (currentEnvironment == MusicEnvironment::mainRoomEnvironment)
	{
		nextTracks = { 0, 1, 2, 3, 4, 5, 6 };
		currentMusic = &mainRoomMusic;
		currentMusic->play();
	}
	else if (currentEnvironment == MusicEnvironment::bangEnvironment)
	{
		currentMusic = &bangBackgroundMusic[GetNextTrack()];
		currentMusic->play();
	}
}


void MusicManager::SetBattleStarted()
{
	if (currentEnvironment == MusicEnvironment::bangEnvironment)
	{
		currentMusic->pause();
		currentBattleMusic = &bangBattleMusic;
		currentBattleMusic->setVolume(50.0f);
		currentBattleMusic->play();
	}
}


void MusicManager::SetBattleEnded()
{
	if (currentEnvironment == MusicEnvironment::bangEnvironment)
	{
		blendInAfterBattleTime = 4.0;
		currentMusic->play();
	}
}


void MusicManager::PlayMusic()
{	
	if (currentBattleMusic != nullptr)
		currentBattleMusic->play();
	else if (currentMusic != nullptr)
		currentMusic->play();
	else
	{
		switch (currentEnvironment)
		{
		case MusicEnvironment::mainRoomEnvironment:
			currentMusic = &mainRoomMusic;
			currentMusic->play();
			break;
		case MusicEnvironment::bangEnvironment:
			currentMusic = &bangBackgroundMusic[GetNextTrack()];
			currentMusic->play();
			break;
		}
	}
}


void MusicManager::StopMusic()
{
	if (currentMusic)
		currentMusic->pause();

	if(currentBattleMusic)
		currentBattleMusic->pause();
}
