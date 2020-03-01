#include "ShopPanel.hpp"

void ShopPanel::Init(CGameEngine *_engine, std::unordered_map<ItemID, bool> &_consumablesAvailability)
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

	for (auto i : items)
		i = nullptr;

	for (int i = 5; i < 15; i++)
	{
		ItemID itemID = static_cast<ItemID>(CONSUMABLE_ITEMS_START + (i - 5));
		if (consumablesAvailability.count(itemID) > 0)
		{
			InventoryItemWrapper *newItem = new InventoryItemWrapper;

			Item rawItem;
			rawItem.id = itemID;
			rawItem.number = -1;

			CSprite newSprite;
			newSprite.Load(g_pTextures->item[itemID]);

			if (consumablesAvailability.at(itemID) == false)
				newSprite.SetColor(40, 40, 40);

			newItem->Init(std::move(rawItem), std::move(newSprite));

			int xPos = ((i-5) % 5) * 116 + shopPanel.GetGlobalRect().left + 33;
			int yPos = shopPanel.GetGlobalRect().top + 288 + 147 * ((i-5) / 5);
			newItem->SetPos(xPos, yPos);

			items[i] = newItem;
		}
	}

	currentlySelectedItem = -1;

	ChooseNewRandomItems(1);
}


void ShopPanel::Update()
{
	for (int i = 0; i < 15; i++)
	{
		if (items[i] != nullptr)
		{
			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) == Pressed)
			{
				if (currentlySelectedItem == i)
					currentlySelectedItem = -1;
				else
				{
					currentlySelectedItem = i;
					selectedItemFrame.SetPos(items[i]->GetGlobalBounds().left - 7, items[i]->GetGlobalBounds().top - 7);
				}
			}
		}
	}
}


void ShopPanel::Render()
{
	shopPanel.Render(engine->GetWindow());

	engine->GetWindow().draw(title);

	int showTooltipForItem = -1;
	for (int i = 0; i < 15; i++)
	{
		if (items[i] != nullptr)
		{
			items[i]->Render(engine->GetWindow());
			if (items[i]->Contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) != Held)
			{
				if(i < 5 || consumablesAvailability.at(items[i]->GetItem().id))
					showTooltipForItem = i;
			}
		}
	}

	if (currentlySelectedItem != -1)
		selectedItemFrame.Render(engine->GetWindow());

	if (showTooltipForItem != -1)
	{
		tooltip.SetItem(items[showTooltipForItem]->GetItem().id);
		tooltip.ShowTooltip(engine->GetWindow(), engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
	}
}


void ShopPanel::Quit()
{
	for (auto i : items)
		SAFE_DELETE(i);
}


void ShopPanel::SetPos(int _x, int _y)
{
	shopPanel.SetPos(_x, _y);
	title.setPosition(shopPanel.GetGlobalRect().left + 140, shopPanel.GetGlobalRect().top + 23);

	for (int i = 0; i < 5; i++)
	{
		if (items[i] != nullptr)
		{
			int xPos = i * 116 + shopPanel.GetGlobalRect().left + 33;
			int yPos = shopPanel.GetGlobalRect().top + 108;
			items[i]->SetPos(xPos, yPos);
		}
	}

	for (int i = 5; i < 15; i++)
	{
		if (items[i] != nullptr)
		{
			int xPos = ((i - 5) % 5) * 116 + shopPanel.GetGlobalRect().left + 33;
			int yPos = shopPanel.GetGlobalRect().top + 288 + 147 * ((i - 5) / 5);
			items[i]->SetPos(xPos, yPos);
		}
	}
}


void ShopPanel::ChooseNewRandomItems(int _level)
{
	for (int i = 0; i < 5; i++)
	{
		InventoryItemWrapper *newItem = new InventoryItemWrapper;

		Item rawItem;
		rawItem.id = GetRandomItemID(_level);
		rawItem.color = GetRandomItemColor();

		CSprite newSprite;
		newSprite.Load(g_pTextures->item[rawItem.id]);

		newItem->Init(std::move(rawItem), std::move(newSprite));

		int xPos = i * 116 + shopPanel.GetGlobalRect().left + 33;
		int yPos = shopPanel.GetGlobalRect().top + 108;
		newItem->SetPos(xPos, yPos);

		items[i] = newItem;
	}
}


sf::Color ShopPanel::GetRandomItemColor()
{
	int randomNumber = rand() % 4;

	if (randomNumber == 0)
		return sf::Color(128, 0, 0);
	else if (randomNumber == 1)
		return sf::Color(0, 128, 0);
	else if (randomNumber == 2)
		return sf::Color(0, 0, 128);
	else
		return sf::Color(128, 128, 0);
}

ItemID ShopPanel::GetRandomItemID(int _level)
{
	int numberOfPossibleItems = 0;
	for (int i = 0; i < _level; i++)
		numberOfPossibleItems += g_pObjectProperties->itemsByLevel[i].size();

	ItemID itemID;
	do {
		int randomNumber = rand() % numberOfPossibleItems;

		int i;
		for (i = 0; i < _level; i++)
		{
			if (randomNumber >= g_pObjectProperties->itemsByLevel[i].size())
				randomNumber -= g_pObjectProperties->itemsByLevel[i].size();
			else
				break;
		}
		itemID = g_pObjectProperties->itemsByLevel[i][randomNumber];
	} while (itemID <= ItemID::dice || itemID >= CONSUMABLE_ITEMS_START);
	
	return itemID;
}
