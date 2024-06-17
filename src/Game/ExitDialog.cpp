#include "ExitDialog.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "Resources\SoundManager.hpp"


ExitDialog::ExitDialog(CGameEngine* _engine)
	: engine(_engine)
	, isOpen(false)
	, panel(g_pTextures->sellMultiplePanel)
	, buttonYes(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Ja")
	, buttonNo(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Nein")
{
	panel.AddText("Speichern und ins Hauptmenü?");
	panel.SetTextFont(0, g_pFonts->f_blackwoodCastle);
	panel.SetTextCharacterSize(0, 40);

	buttonYes.SetButtontextFont(g_pFonts->f_trajan);
	buttonYes.SetButtontextCharactersize(23);
	buttonYes.SetButtontextStyle(sf::Text::Bold);
	buttonYes.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonNo.SetButtontextFont(g_pFonts->f_trajan);
	buttonNo.SetButtontextCharactersize(23);
	buttonNo.SetButtontextStyle(sf::Text::Bold);
	buttonNo.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	SetPos(engine->GetWindowSize().x / 2 - panel.GetRect().width / 2, engine->GetWindowSize().y / 2 - panel.GetRect().height / 2);
}

bool ExitDialog::Update()
{
	if (isOpen)
	{
		if (buttonYes.Update(*engine))
			return true;

		if (buttonNo.Update(*engine))
		{
			isOpen = false;
		}
	}

	return false;
}

void ExitDialog::Render()
{
	if (isOpen)
	{
		panel.Render(engine->GetRenderTarget());
		buttonYes.Render(engine->GetRenderTarget());
		buttonNo.Render(engine->GetRenderTarget());
	}
}


void ExitDialog::SetPos(int _x, int _y)
{
	panel.SetPos(_x, _y);
	buttonYes.SetPos(_x + 50, _y + 205);
	buttonNo.SetPos(_x + 334, _y + 205);

	panel.SetTextPos(0, 0, 100);
	panel.SetTextPosXCentered(0);
}
