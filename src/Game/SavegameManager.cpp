#include "SavegameManager.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

const std::string SavegameManager::GetSavegameName(int _index)
{
    for (const auto& entry : fs::directory_iterator("Data/Saves"))
    {
        if (entry.path().filename().string().starts_with(std::to_string(_index))) {
            auto path = entry.path().filename().string();
            return path.substr(2);
        }
    }

    return "";
}

GameStatus* SavegameManager::LoadSavegame(int _index)
{
    auto name = GetSavegameName(_index);
    auto path = "Data/Saves/" + std::to_string(_index) + "_" + name;
    auto status = new GameStatus();
    status->Init();

    std::ifstream input(path);

    status->path = path;
    input >> status->cards;
    input >> status->dice;
    input >> status->levels[0];
    input >> status->levels[1];
    input >> status->levels[2];

    bool skill;
    for (int p = 0; p < 4; p++) {
        for (int a = 0; a < 4; a++) {
            for (int s = 0; s < 8; s++) {
                input >> skill;
                if(skill == true)
                    status->AcquireSkill(p, a, s);
            }
        }
    }
   
    int currentID;
    int r, g, b;
    input >> currentID;
    while (currentID != ItemID::empty) {
        Item item;
        input >> r;
        input >> g;
        input >> b;
        item.color.r = static_cast<sf::Uint8>(r);
        item.color.g = static_cast<sf::Uint8>(g);
        item.color.b = static_cast<sf::Uint8>(b);
        input >> item.number;
        item.id = static_cast<ItemID>(currentID);
        status->AddItem(item);
        input >> currentID;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            Item item;
            int id;
            input >> id;
            input >> r;
            input >> g;
            input >> b;
            item.color.r = static_cast<sf::Uint8>(r);
            item.color.g = static_cast<sf::Uint8>(g);
            item.color.b = static_cast<sf::Uint8>(b);
            input >> item.number;
            item.id = static_cast<ItemID>(id);
            if (item.id != ItemID::empty) {
                status->AddEquipment(i, j, item);
            }
        }
    }

    input >> currentID;
    while (currentID != ItemID::empty) {
        bool available;
        input >> available;
        status->consumablesAvailability[static_cast<ItemID>(currentID)] = available;
        input >> currentID;
    }

    input.close();
    return status;
}

void SavegameManager::StoreSavegame(GameStatus* _status)
{
    std::ofstream output(_status->path);

    output << _status->cards << "\n";
    output << _status->dice << "\n";
    output << _status->levels[0] << "\n";
    output << _status->levels[1] << "\n";
    output << _status->levels[2] << "\n";

    for(auto &player : _status->skillAcquired)
        for(auto &ability : player)
            for(auto &skill : ability)
                output << skill << "\n";

    for (auto& item : _status->items) {
        output << static_cast<int>(item.id) << "\n";
        output << static_cast<int>(item.color.r) << "\n";
        output << static_cast<int>(item.color.g) << "\n";
        output << static_cast<int>(item.color.b) << "\n";
        output << item.number << "\n";
    }

    for(auto &item : _status->consumables) {
        output << static_cast<int>(item.id) << "\n";
        output << static_cast<int>(item.color.r) << "\n";
        output << static_cast<int>(item.color.g) << "\n";
        output << static_cast<int>(item.color.b) << "\n";
        output << item.number << "\n";
    }
    output << static_cast<int>(ItemID::empty) << "\n";

    for (auto& player : _status->equipment) {
        for(auto &item : player) {
            output << static_cast<int>(item.id) << "\n";
            output << static_cast<int>(item.color.r) << "\n";
            output << static_cast<int>(item.color.g) << "\n";
            output << static_cast<int>(item.color.b) << "\n";
            output << item.number << "\n";
        }
    }

    for (auto& item : _status->consumablesAvailability) {
        output << static_cast<int>(item.first) << "\n";
        output << item.second << "\n";
    }

    output << static_cast<int>(ItemID::empty);

    output.close();
}


void SavegameManager::DeleteSavegame(int _index)
{
    for (const auto& entry : fs::directory_iterator("Data/Saves"))
    {
        if (entry.path().filename().string().starts_with(std::to_string(_index))) {
            fs::remove(entry.path());
            return;
        }
    }
}
