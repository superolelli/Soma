#include "Lootable.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

Lootable::Lootable(LootableID _id, DialogManager *_dialogManager)
	: id(_id)
	, dialogManager(_dialogManager)
	, sprite(g_pTextures->lootable[_id], 2, g_pTextures->lootable[_id].getSize().x / 2, g_pTextures->lootable[_id].getSize().y)
	, currentBoundingBox(g_pObjectProperties->lootableProperties[_id].boundingBox)
	, wasLooted(false)
{
}


void Lootable::AddItem(Item const &_item)
{
	items.push_back(_item);
}

void Lootable::SetPos(int _x, int _y)
{
	sprite.SetPos(_x, _y);
	currentBoundingBox.left = _x + g_pObjectProperties->lootableProperties[id].boundingBox.left;
	currentBoundingBox.top = _y + g_pObjectProperties->lootableProperties[id].boundingBox.top;
}

void Lootable::Render(sf::RenderTarget &_target)
{
	if (wasLooted)
		sprite.Render(_target, 1.0f);
	else
		sprite.Render(_target, 0.0f);
}


void Lootable::Update(CGameEngine *_engine)
{
	if (!wasLooted && currentBoundingBox.contains(_engine->GetWorldMousePos()))
	{
		_engine->SetCursor(sf::Cursor::Type::Hand);

		if (_engine->GetButtonstates(ButtonID::Left) == Keystates::Released)
		{
			LootableDialog *dialog = new LootableDialog(_engine, id);

			for(auto &i : items)
				dialog->AddItem(i);

			dialogManager->AddLootableDialog(dialog);

			wasLooted = true;
			g_pSounds->PlaySound(lootableSounds[id]);
		}
	}
}