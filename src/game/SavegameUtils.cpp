#include "SavegameUtils.hpp"
#include <filesystem>

namespace fs = std::filesystem;

std::string SavegameUtils::SAVEGAME_PATH = "Data/Saves/";

void SavegameUtils::InitSavegameDirectory()
{
    fs::create_directory(SAVEGAME_PATH);
}

const std::string SavegameUtils::GetSavegameName(int _index)
{
    for (const auto& entry : fs::directory_iterator(SAVEGAME_PATH))
    {
        if (entry.path().filename().string().starts_with(std::to_string(_index))) {
            auto path = entry.path().filename().string();
            return path.substr(2);
        }
    }

    return "";
}

const std::string SavegameUtils::GetSavegamePath(int _index)
{
    return SAVEGAME_PATH + std::to_string(_index) + "_" + GetSavegameName(_index);
}


void SavegameUtils::DeleteSavegame(int _index)
{
    for (const auto& entry : fs::directory_iterator(SAVEGAME_PATH))
    {
        if (entry.path().filename().string().starts_with(std::to_string(_index))) {
            fs::remove(entry.path());
            return;
        }
    }
}
