#include "NameInputPanel.hpp"
#include "Resources/FontManager.hpp"
#include "Resources/TextureManager.hpp"

NameInputPanel::NameInputPanel(const std::string &_initialString)
    : nameInput(g_pFonts->f_blackwoodCastle, 50)
    , finishedButton(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Fertig")
    , backgroundSprite(g_pTextures->nameInputBackground)
{
    nameInput.SetString(_initialString);
    nameInput.SetMaximumCharacters(20);

    finishedButton.SetButtontextFont(g_pFonts->f_blackwoodCastle);

    visible = false;
}

bool NameInputPanel::Update(CGameEngine *_engine)
{
    if (visible) {
        nameInput.HandleInput(*_engine);

        if (finishedButton.Update(*_engine))
            return true;
    }
    return false;
}

void NameInputPanel::Render(sf::RenderTarget &_target)
{
    if (visible) {
        backgroundSprite.Render(_target);
        nameInput.Render(_target);
        finishedButton.Render(_target);
    }
}

void NameInputPanel::SetPos(int _x, int _y)
{
    backgroundSprite.SetPos(_x, _y);
    nameInput.SetPos(_x + backgroundSprite.GetRect().width/2, _y + 15);
    finishedButton.SetPos(_x + backgroundSprite.GetRect().width / 2 - finishedButton.GetRect().width / 2, _y + backgroundSprite.GetRect().height + 10);
}

void NameInputPanel::SetVisible(bool _visible)
{
    visible = _visible;
}


const std::string& NameInputPanel::GetString()
{
    return nameInput.GetString();
}