#pragma once

#include "GameStatus.hpp"

class SavegameManager {
public:

    static const std::string GetSavegameName(int _index);

    static GameStatus* LoadSavegame(int _index);
    static void StoreSavegame(GameStatus* _status);
    static void DeleteSavegame(int _index);

private:


};