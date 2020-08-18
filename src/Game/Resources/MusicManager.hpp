#pragma once

#include "../../Framework/Patterns/Singleton.hpp"
#include <SFML\Audio.hpp>

enum MusicEnvironment {mainRoomEnvironment, bangEnvironment, kutschfahrtEnvironment, tichuEnvironment};

#define g_pMusic MusicManager::Get()
class MusicManager : public TSingleton<MusicManager> {
public:

	void LoadMusic();

	void SetCurrentEnvironment(MusicEnvironment _environmentID);
	void SetBattleStarted();
	void SetBattleEnded();
	void PlayMusic();
	void StopMusic();

	void Update();

private:

	int GetNextTrack();

	sf::Music backgroundMusic[2][7];
	sf::Music bangBattleMusic;

	sf::Music mainRoomMusic;

	MusicEnvironment currentEnvironment;

	sf::Music *currentMusic;
	sf::Music *currentBattleMusic;

	std::vector<int> nextTracks;

	double blendInAfterBattleTime;
};