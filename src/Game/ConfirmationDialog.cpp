#include "ConfirmationDialog.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "Resources\SoundManager.hpp"

ConfirmationDialog::ConfirmationDialog(CGameEngine* _engine, const sf::String &_text, int _fontSize, int _id)
	: engine(_engine)
	, isOpen(false)
	, yesChosen(false)
	, confirmationID(_id)
	, panel(g_pTextures->confirmationDialogPanel)
	, buttonYes(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Ja")
	, buttonNo(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Nein")
{
	buttonYes.SetButtontextFont(g_pFonts->f_trajan);
	buttonYes.SetButtontextCharactersize(23);
	buttonYes.SetButtontextStyle(sf::Text::Bold);
	buttonYes.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonNo.SetButtontextFont(g_pFonts->f_trajan);
	buttonNo.SetButtontextCharactersize(23);
	buttonNo.SetButtontextStyle(sf::Text::Bold);
	buttonNo.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	panel.AddText(_text);
	panel.SetTextCharacterSize(0, _fontSize);
	panel.SetTextFont(0, g_pFonts->f_trajan);
	panel.SetTextStyle(0, sf::Text::Style::Bold);
	panel.SetTextColor(0, sf::Color::Black);
	panel.SetTextPos(0, 50, 40);
	panel.SetTextPosXCentered(0);

	SetPos(engine->GetWindowSize().x / 2 - panel.GetRect().width / 2, engine->GetWindowSize().y / 2 - panel.GetRect().height / 2);
}


void ConfirmationDialog::Update()
{
	if (isOpen)
	{
		if (buttonNo.Update(*engine))
		{
			yesChosen = false;
			isOpen = false;
		}

		if (buttonYes.Update(*engine))
		{
			yesChosen = true;
			isOpen = false;
		}
	}
}

void ConfirmationDialog::Render()
{
	if (isOpen)
	{
		panel.Render(engine->GetRenderTarget());
		buttonYes.Render(engine->GetRenderTarget());
		buttonNo.Render(engine->GetRenderTarget());
	}
}

void ConfirmationDialog::SetPos(int _x, int _y)
{
	panel.SetPos(_x, _y);
	buttonYes.SetPos(_x + 40, _y + 132);
	buttonNo.SetPos(_x + 334, _y + 132);
}
