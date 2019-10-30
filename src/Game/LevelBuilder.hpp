#pragma once

#include "Level.hpp"


class LevelBuilder {
public:

	static Level *buildLevel (int _levelID, DialogManager *_dialogManager, GameStatus *_gameStatus);
};