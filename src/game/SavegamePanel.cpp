#include "SavegamePanel.hpp"
#include "Resources/TextureManager.hpp"
#include "Resources/FontManager.hpp"

SavegamePanel::SavegamePanel(CGameEngine *_engine, const std::string &_name)
    : engine(_engine)
    , clicked(false)
    , empty(_name == "")
    , panel(g_pTextures->savegamePanel)
    , continueGameButton(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Fortsetzen")
    , deleteGameButton(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "L�schen")
    , newGameButton(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Neues Spiel")
{
    auto textID = panel.AddText(empty ? "Empty" : _name);
    panel.SetTextFont(textID, g_pFonts->f_blackwoodCastle);
    panel.SetTextCharacterSize(textID, 40);
    panel.SetTextPosCentered(textID);

    continueGameButton.SetButtontextFont(g_pFonts->f_blackwoodCastle);
    deleteGameButton.SetButtontextFont(g_pFonts->f_blackwoodCastle);
    newGameButton.SetButtontextFont(g_pFonts->f_blackwoodCastle);
}

SavegamePanel::SavegameState SavegamePanel::Update()
{
    if (clicked && empty) {
        if (newGameButton.Update(*engine))
            return SavegameState::Create;
    }

    if (clicked && !empty) {
        if (continueGameButton.Update(*engine))
            return SavegameState::Continue;

        if (deleteGameButton.Update(*engine)) {
            panel.ChangeString(0, "");
            panel.SetTextPosCentered(0);
            empty = true;
            return SavegameState::Delete;
        }
    }

    if (engine->GetButtonstates(ButtonID::Left) == Keystates::Released) 
        clicked = panel.GetRect().contains(engine->GetMousePos());

    return SavegameState::Nothing;
}

void SavegamePanel::Render()
{
    panel.Render(engine->GetRenderTarget());

    if (clicked && empty) 
        newGameButton.Render(engine->GetRenderTarget());
    else if (clicked && !empty) 
    {
        continueGameButton.Render(engine->GetRenderTarget());
        deleteGameButton.Render(engine->GetRenderTarget());
    }
}

void SavegamePanel::SetPos(int _x, int _y)
{
    panel.SetPos(_x, _y);
    newGameButton.SetPos(_x + panel.GetRect().size.x/2 - newGameButton.GetRect().size.x / 2, _y + panel.GetRect().size.y + 20);
    continueGameButton.SetPos(_x + panel.GetRect().size.x / 2 - continueGameButton.GetRect().size.x / 2, _y + panel.GetRect().size.y + 20);
    deleteGameButton.SetPos(_x + panel.GetRect().size.x / 2 - deleteGameButton.GetRect().size.x / 2, continueGameButton.GetRect().position.y + continueGameButton.GetRect().size.y + 20);
}

void SavegamePanel::SetName(const std::string& _name)
{
    empty = (_name == "");
    panel.ChangeString(0, _name);
    panel.SetTextPosCentered(0);
}
