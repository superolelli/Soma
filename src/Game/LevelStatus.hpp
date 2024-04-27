#pragma once

#include "../Framework/Patterns/Observer.hpp"
#include <set>
#include "Item.hpp"

const int MAX_FATIGUE_VALUE = 100;

class LevelStatus : public CObserver {

public:

	LevelStatus(int _difficulty);

	void AddFatigue(int _fatigue);
	void RemoveFatigue(int _fatigue);

	void OpponentKilled();
	void PlayerDied();

	// For fatigue bar
	int* GetFatiguePtr() { return &fatigue; }
	int* GetMaxFatiguePtr() { return &maxFatigue; }
	float GetRelativeFatigue() const;

	const std::set<ItemID>& GetUnlockedItems() const { return unlockedItems; }

	virtual void OnNotify(ObserverNotification const& _notification) override;

	std::set<ItemID> unlockedItems;

	int difficulty;

	int fatigue;
	int maxFatigue;

	int opponentsKilled;
	int playersDied;
};