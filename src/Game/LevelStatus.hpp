#pragma once

const int MAX_FATIGUE_VALUE = 100;

class LevelStatus {

public:

	void Reset(int _difficulty);

	void AddFatigue(int _fatigue);
	void RemoveFatigue(int _fatigue);
	void ResetFatigue();

	void OpponentKilled();
	void PlayerDied();

	// For fatigue bar
	int* GetFatiguePtr() { return &fatigue; }
	int* GetMaxFatiguePtr() { return &maxFatigue; }
	float GetRelativeFatigue() const;

	int difficulty;

	int fatigue;
	int maxFatigue;

	int opponentsKilled;
	int playersDied;
};