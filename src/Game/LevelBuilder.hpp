#pragma once

#include "Level.hpp"


class LevelBuilder {
public:

	static Level *buildLevel (LevelType _levelType, int _levelID, DialogManager *_dialogManager);
};