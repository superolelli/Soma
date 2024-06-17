#include "SellMultipleDialog.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\FontManager.hpp"
#include "Resources\SoundManager.hpp"

SellMultipleDialog::SellMultipleDialog(CGameEngine * _engine)
	: engine(_engine)
	, isOpen(true)
	, amount(0)
	, sellMultiplePanel(g_pTextures->sellMultiplePanel)
	, slider(g_pTextures->sellMultipleSlider, g_pTextures->sellMultipleKnob)
	, buttonSell(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Verkaufen")
	, buttonCancel(g_pTextures->bangGenericButton, Buttontypes::Motion_Up, "Abbrechen")
{
	slider.SetFont(g_pFonts->f_kingArthur);
	slider.SetMaximum(10);
	slider.ShowText(true);

	buttonSell.SetButtontextFont(g_pFonts->f_trajan);
	buttonSell.SetButtontextCharactersize(23);
	buttonSell.SetButtontextStyle(sf::Text::Bold);
	buttonSell.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonCancel.SetButtontextFont(g_pFonts->f_trajan);
	buttonCancel.SetButtontextCharactersize(23);
	buttonCancel.SetButtontextStyle(sf::Text::Bold);
	buttonCancel.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	SetPos(engine->GetWindowSize().x / 2 - sellMultiplePanel.GetRect().width / 2, engine->GetWindowSize().y / 2 - sellMultiplePanel.GetRect().height / 2);
}

SellMultipleDialog::~SellMultipleDialog()
{
}

void SellMultipleDialog::Update()
{
	if (isOpen)
	{
		if (buttonCancel.Update(*engine))
			isOpen = false;

		if (buttonSell.Update(*engine))
		{
			amount = slider.GetCurrentValue();
			isOpen = false;
		}

		slider.Update(*engine);
	}
}

void SellMultipleDialog::Render()
{
	if (isOpen)
	{
		sellMultiplePanel.Render(engine->GetRenderTarget());
		slider.Render(engine->GetRenderTarget());
		buttonSell.Render(engine->GetRenderTarget());
		buttonCancel.Render(engine->GetRenderTarget());
	}
}

void SellMultipleDialog::SetItem(Item const & _item)
{
	slider.SetMaximum(_item.number);
}

void SellMultipleDialog::SetPos(int _x, int _y)
{
	sellMultiplePanel.SetPos(_x, _y);
	slider.SetPos(_x + 50, _y + 100);
	buttonSell.SetPos(_x + 50, _y + 205);
	buttonCancel.SetPos(_x + 334, _y + 205);
}
