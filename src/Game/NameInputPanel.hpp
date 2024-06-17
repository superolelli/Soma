#pragma once

#include "../Framework/Gui/StringInput.hpp"
#include "../Framework/Gui/Button.hpp"

class NameInputPanel {
public:

    NameInputPanel(const std::string& _initialString);

    bool Update(CGameEngine *_engine);
    void Render(sf::RenderTarget &_target);

    void SetPos(int _x, int _y);

    void SetVisible(bool _visible);

    const std::string& GetString();

    sf::IntRect GetRect() { return backgroundSprite.GetRect(); }

private:

    bool visible;

    CStringInput nameInput;
    CButton finishedButton;
    CSprite backgroundSprite;
};