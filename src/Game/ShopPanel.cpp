#include "ShopPanel.hpp"
#include "ItemFactory.hpp"

void ShopPanel::Init(CGameEngine *_engine, std::unordered_map<ItemID, bool> *_consumablesAvailability)
{
	engine = _engine;
	consumablesAvailability = _consumablesAvailability;

	tooltip.Init();

	shopPanel.Load(g_pTextures->shopPanel);

	selectedItemFrame.Load(g_pTextures->selectedItemFrame);

	title.setCharacterSize(50);
	title.setFont(g_pFonts->f_blackwoodCastle);
	title.setFillColor(sf::Color::Black);
	title.setString("Nur für kurze Zeit!");

	priceSign.Load(g_pTextures->priceSign);
	priceSign.AddText("");
	priceSign.SetTextCharacterSize(0, 14);
	priceSign.SetTextFont(0, g_pFonts->f_kingArthur);
	priceSign.SetTextColor(0, sf::Color::White);

	for (auto i : items)
		i = nullptr;

	for (int i = 5; i < 15; i++)
	{
		ItemID itemID = static_cast<ItemID>(CONSUMABLE_ITEMS_START + (i - 5));
		if (consumablesAvailability->count(itemID) > 0)
		{
			InventoryItemWrapper *newItem = new InventoryItemWrapper;

			Item rawItem;
			rawItem.id = itemID;
			rawItem.number = -1;

			CSprite newSprite;
			newSprite.Load(g_pTextures->item[itemID]);

			if (consumablesAvailability->at(itemID) == false)
				newSprite.SetColor(40, 40, 40);

			newItem->Init(std::move(rawItem), std::move(newSprite));

			int xPos = ((i-5) % 5) * 116 + shopPanel.GetGlobalRect().left + 33;
			int yPos = shopPanel.GetGlobalRect().top + 297 + 144 * ((i-5) / 5);
			newItem->SetPos(xPos, yPos);

			items[i] = newItem;
		}
	}

	currentlySelectedItem = -1;
}


void ShopPanel::Update()
{
	if (engine->GetButtonstates(ButtonID::Left) == Pressed)
	{
		for (int i = 0; i < 15; i++)
		{
			if (items[i] != nullptr)
			{
				if (items[i]->Contains(engine->GetMousePos()))
				{
					if (currentlySelectedItem == i)
						currentlySelectedItem = -1;
					else
					{
						currentlySelectedItem = i;
						selectedItemFrame.SetPos(items[i]->GetGlobalBounds().left - 7, items[i]->GetGlobalBounds().top - 7);
						OnItemSelected(items[currentlySelectedItem]->GetItem());
					}
				}
			}
		}
	}
}


void ShopPanel::Render()
{
	shopPanel.Render(engine->GetRenderTarget());

	engine->GetRenderTarget().draw(title);

	int showTooltipForItem = -1;
	for (int i = 0; i < 15; i++)
	{
		if (items[i] != nullptr)
		{
			items[i]->Render(engine->GetRenderTarget());

			if(currentlySelectedItem == i)
				selectedItemFrame.Render(engine->GetRenderTarget());

			priceSign.ChangeString(0, std::to_string(g_pObjectProperties->getItemStats(items[i]->GetItem().id).price));
			priceSign.SetPos(items[i]->GetGlobalBounds().left + 18, items[i]->GetGlobalBounds().top + 90);
			priceSign.SetTextPosCentered(0);
			priceSign.Render(engine->GetRenderTarget());

			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
			{
				if(i < 5 || consumablesAvailability->at(items[i]->GetItem().id))
					showTooltipForItem = i;
			}
		}
	}

	if (showTooltipForItem != -1)
	{
		tooltip.SetItem(items[showTooltipForItem]->GetItem().id);
		tooltip.ShowTooltip(engine->GetRenderTarget(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
	}
}


void ShopPanel::Quit()
{
	for (auto i : items)
		SAFE_DELETE(i);
}

void ShopPanel::SetOnItemSelectedCallback(std::function<void(Item&)> _onItemSelected)
{
	OnItemSelected = _onItemSelected;
}


void ShopPanel::SetPos(int _x, int _y)
{
	shopPanel.SetPos(_x, _y);
	title.setPosition(shopPanel.GetGlobalRect().left + 140, shopPanel.GetGlobalRect().top + 25);

	for (int i = 0; i < 5; i++)
	{
		if (items[i] != nullptr)
		{
			int xPos = i * 116 + shopPanel.GetGlobalRect().left + 33;
			int yPos = shopPanel.GetGlobalRect().top + 114;
			items[i]->SetPos(xPos, yPos);
		}
	}

	for (int i = 5; i < 15; i++)
	{
		if (items[i] != nullptr)
		{
			int xPos = ((i - 5) % 5) * 116 + shopPanel.GetGlobalRect().left + 33;
			int yPos = shopPanel.GetGlobalRect().top + 297 + 144 * ((i - 5) / 5);
			items[i]->SetPos(xPos, yPos);
		}
	}
}


void ShopPanel::ChooseNewRandomItems(int _bangLevel, int _kutschfahrtLevel, int _tichuLevel)
{
	for (int i = 0; i < 5; i++)
	{
		InventoryItemWrapper *newItem = new InventoryItemWrapper;

		Item rawItem = ItemFactory::CreateShopItem(_bangLevel, _kutschfahrtLevel);

		CSprite newSprite;
		newSprite.Load(g_pTextures->item[rawItem.id]);

		newItem->Init(std::move(rawItem), std::move(newSprite));

		int xPos = i * 116 + shopPanel.GetGlobalRect().left + 33;
		int yPos = shopPanel.GetGlobalRect().top + 114;
		newItem->SetPos(xPos, yPos);

		items[i] = newItem;
	}

	for (int i = 5; i < 15; i++)
	{
		if (items[i] != nullptr)
		{
			if (consumablesAvailability->at(items[i]->GetItem().id) == true)
				items[i]->SetSpriteColor(255, 255, 255);
			else
				items[i]->SetSpriteColor(40, 40, 40);
		}
	}
}


Item ShopPanel::RetrieveCurrentlySelectedItem()
{
	if (currentlySelectedItem != -1)
	{
		if (items[currentlySelectedItem]->GetItem().id < CONSUMABLE_ITEMS_START || consumablesAvailability->at(items[currentlySelectedItem]->GetItem().id))
		{
			auto item = items[currentlySelectedItem]->GetItem();

			if (currentlySelectedItem < 5)
			{
				SAFE_DELETE(items[currentlySelectedItem]);
				currentlySelectedItem = -1;
			}

			item.number = 1;
			return item;
		}
	}

	Item item;
	item.id = ItemID::empty;
	return item;
}


bool ShopPanel::IsItemSelected()
{
	if (currentlySelectedItem != -1)
	{
		if (items[currentlySelectedItem]->GetItem().id < CONSUMABLE_ITEMS_START || consumablesAvailability->at(items[currentlySelectedItem]->GetItem().id))
			return true;
	}
	return false;
}

int ShopPanel::CurrentItemPrice()
{
	if (currentlySelectedItem != -1)
		return g_pObjectProperties->getItemStats(items[currentlySelectedItem]->GetItem().id).price;
	else
		return 0;
}