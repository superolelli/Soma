#pragma once

#include "../Framework/Graphics/SpriteWithText.hpp"
#include "../Framework/Gui/Button.hpp"



class SavegamePanel {
public:

    enum class SavegameState { Nothing, Create, Continue, Delete };

    void Init(CGameEngine* _engine, const std::string& _name = "");
    SavegameState Update();
    void Render();

    void SetPos(int _x, int _y);
    void SetName(const std::string& _name);

private:

    CGameEngine* engine;

    bool clicked;
    bool empty;

    SpriteWithText panel;
    CButton continueGameButton;
    CButton deleteGameButton;
    CButton newGameButton;
};
