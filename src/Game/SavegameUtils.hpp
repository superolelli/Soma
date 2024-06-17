#pragma once

#include <string>

class SavegameUtils {
public:

    static std::string SAVEGAME_PATH;

    static const std::string GetSavegameName(int _index);
    static const std::string GetSavegamePath(int _index);
    
    static void InitSavegameDirectory();
    static void DeleteSavegame(int _index);

private:


};