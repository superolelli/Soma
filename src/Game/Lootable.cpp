#include "Lootable.hpp"
#include "Resources\TextureManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "Resources\SoundManager.hpp"

void Lootable::Init(LootableID _id)
{
	id = _id;

	sprite.Load(g_pTextures->lootable[_id], 2, g_pTextures->lootable[_id].getSize().x / 2, g_pTextures->lootable[_id].getSize().y);

	currentBoundingBox = g_pObjectProperties->lootablesBoundingBoxes[id];
	wasLooted = false;
}


void Lootable::SetPos(int _x, int _y)
{
	sprite.SetPos(_x, _y);
	currentBoundingBox.left = _x + g_pObjectProperties->lootablesBoundingBoxes[id].left;
	currentBoundingBox.top = _y + g_pObjectProperties->lootablesBoundingBoxes[id].top;
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
			wasLooted = true;
			g_pSounds->PlaySound(soundID(id + LOOTABLE_SOUNDS_START));
		}
	}
}