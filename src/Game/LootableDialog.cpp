#include "LootableDialog.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\ObjectPropertiesManager.hpp"

void LootableDialog::Init(CGameEngine *_engine, GameStatus * _gameStatus, int _lootableID)
{
	engine = _engine;
	gameStatus = _gameStatus;

	lootablePanel.Load(g_pTextures->lootablePanel);

	buttonClose.Load(g_pTextures->lootablePanelCloseButton, Buttontypes::Motion_Up);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonTakeAll.Load(g_pTextures->lootablePanelButton, Buttontypes::Motion_Up, "  Alles\nnehmen");
	buttonTakeAll.SetButtontextFont(g_pFonts->f_trajan);
	buttonTakeAll.SetButtontextCharactersize(23);
	buttonTakeAll.SetButtontextStyle(sf::Text::Bold);
	buttonTakeAll.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	lootableNameText.setCharacterSize(20);
	lootableNameText.setFont(g_pFonts->f_trajan);
	lootableNameText.setFillColor(sf::Color::Black);
	lootableNameText.setStyle(sf::Text::Bold);
	lootableNameText.setString(g_pObjectProperties->lootableProperties[_lootableID].name);

	SetPos(engine->GetWindowSize().x / 2 - lootablePanel.GetGlobalRect().width / 2, 200);

	tooltip.Init();

	isOpen = true;
}


void LootableDialog::Quit()
{
	for (int i = 0; i < 9; i++)
		SAFE_DELETE(items[i]);
}

void LootableDialog::SetTitle(std::string &_str)
{
	lootableNameText.setString(_str);
}

void LootableDialog::SetPos(int _x, int _y)
{
	lootablePanel.SetPos(_x, _y);
	buttonClose.SetPos(lootablePanel.GetGlobalRect().left + 522, lootablePanel.GetGlobalRect().top + 65);
	buttonTakeAll.SetPos(lootablePanel.GetGlobalRect().left + 423, lootablePanel.GetGlobalRect().top + 342);
	lootableNameText.setPosition(lootablePanel.GetGlobalRect().left + 467 - lootableNameText.getGlobalBounds().width / 2, lootablePanel.GetGlobalRect().top + 15);
}

void LootableDialog::Update()
{
	if (isOpen)
	{
		if (buttonClose.Update(*engine))
			isOpen = false;

		if (buttonTakeAll.Update(*engine))
		{
			TakeAllItems();
			isOpen = false;
		}


		if (engine->GetButtonstates(ButtonID::Left) == Released)
		{
			for (int i = 0; i < 9; i++)
			{
				if (items[i] == nullptr)
					continue;

				if (items[i]->Contains(engine->GetMousePos()))
				{
					TakeItem(i);
					g_pSounds->PlaySound(soundID::INVENTORY_DROP);
				}
			}
		}
	}
}

void LootableDialog::Render()
{
	if (isOpen)
	{
		lootablePanel.Render(engine->GetWindow());

		for (auto i : items)
		{
			if(i != nullptr)
				i->Render(engine->GetWindow());
		}

		buttonClose.Render(engine->GetWindow());
		buttonTakeAll.Render(engine->GetWindow());

		engine->GetWindow().draw(lootableNameText);

		for (auto i : items)
		{
			if (i == nullptr)
				continue;

			if (i->Contains(engine->GetMousePos()))
			{
				tooltip.SetItemID(i->GetItem().id);
				tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
				break;
			}
		}
	}
}


void LootableDialog::AddItem(Item &_item)
{
	int freeSlot = GetFreeItemSlot();
	if (freeSlot == -1)
		return;
	
	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);

	InventoryItemWrapper *newItem = new InventoryItemWrapper;
	newItem->Init(std::move(_item), std::move(newSprite));

	int xPos = (freeSlot % 3) * 116 + lootablePanel.GetGlobalRect().left + 46;
	int yPos = (freeSlot / 3) * 116 + lootablePanel.GetGlobalRect().top + 92;
	newItem->SetPos(xPos, yPos);

	items[freeSlot] = newItem;
}


void LootableDialog::TakeAllItems()
{
	g_pSounds->PlaySound(soundID::LOOTABLE_TAKE_ALL);
	for (int i = 0; i < 9; i++)
	{
		if (items[i] != nullptr)
			TakeItem(i);
	}
}


void LootableDialog::TakeItem(int _id)
{
	gameStatus->AddItem(items[_id]->GetItem());
	SAFE_DELETE(items[_id]);
}

int LootableDialog::GetFreeItemSlot()
{
	for (int i = 0; i < 9; i++)
	{
		if (items[i] == nullptr)
			return i;
	}

	return -1;
}