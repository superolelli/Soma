#include "Inventory.hpp"
#include "Resources\SoundManager.hpp"
#include "Resources\StringManager.hpp"

void Inventory::Init(GameStatus * _gameStatus, CGameEngine * _engine)
{
	gameStatus = _gameStatus;
	gameStatus->SetOnItemAddedCallback([&](Item _item) {OnItemAdded(_item); });
	engine = _engine;

	currentPlayer = 0;
	currentUpperRow = 0;

	inventoryPanel.Load(g_pTextures->inventoryPanel);
	inventoryPanel.SetPos(150, 70);

	scrollbar.Init(35, 563);
	scrollbar.SetColor(sf::Color(28, 28, 28));
	scrollbar.SetPos(inventoryPanel.GetGlobalRect().left + 1423, inventoryPanel.GetGlobalRect().top + 200);

	buttonNext.Load(g_pTextures->skillPanelButtonNext, Buttontypes::Up);
	buttonNext.SetPos(inventoryPanel.GetGlobalRect().left + 217, inventoryPanel.GetGlobalRect().top + 66);
	buttonNext.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonPrevious.Load(g_pTextures->skillPanelButtonPrevious, Buttontypes::Up);
	buttonPrevious.SetPos(inventoryPanel.GetGlobalRect().left + 15, inventoryPanel.GetGlobalRect().top + 66);
	buttonPrevious.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonClose.Load(g_pTextures->skillPanelButtonClose, Buttontypes::Motion_Up);
	buttonClose.SetPos(inventoryPanel.GetGlobalRect().left + 1475, inventoryPanel.GetGlobalRect().top + 66);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	currentPlayerName.setCharacterSize(47);
	currentPlayerName.setFont(g_pFonts->f_blackwoodCastle);
	currentPlayerName.setFillColor(sf::Color::Black);
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(inventoryPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, inventoryPanel.GetGlobalRect().top + 55);

	panelTitle.setCharacterSize(70);
	panelTitle.setFont(g_pFonts->f_blackwoodCastle);
	panelTitle.setFillColor(sf::Color::Black);
	panelTitle.setString("Inventar");
	panelTitle.setPosition(inventoryPanel.GetGlobalRect().left + 850, inventoryPanel.GetGlobalRect().top + 70);

	tooltip.Init(engine);

	closed = true;
}


void Inventory::Update()
{
	if (!closed)
	{
		if (buttonClose.Update(*engine) == true)
			closed = true;

		scrollbar.Update(*engine);

		if (scrollbar.GetCurrentStep() != currentUpperRow)
		{
			currentUpperRow = scrollbar.GetCurrentStep();
			RecalculatePositionsOfItems();
		}

		CheckButtonsForPlayerChoosing();
	}
}


void Inventory::RecalculatePositionsOfItems()
{
	int yPos = inventoryPanel.GetGlobalRect().top + 195;

	for (int y = 0; y < 5; y++)
	{
		int xPos = inventoryPanel.GetGlobalRect().left + 832;
		for (int x = 0; x < 5; x++)
		{
			items[(currentUpperRow + y) * 5 + x].second.SetPos(xPos, yPos);
			xPos += 116;
		}
		yPos += 116;
	}
}


void Inventory::UpdateGUIForChosenPlayer()
{
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(inventoryPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, inventoryPanel.GetGlobalRect().top + 55);
}

void Inventory::CheckButtonsForPlayerChoosing()
{
	int lastCurrentPlayer = currentPlayer;

	if (buttonNext.Update(*engine) == true)
		currentPlayer++;

	if (buttonPrevious.Update(*engine) == true)
		currentPlayer--;

	if (currentPlayer > 3)
		currentPlayer = 0;
	if (currentPlayer < 0)
		currentPlayer = 3;

	if (currentPlayer != lastCurrentPlayer)
	{
		UpdateGUIForChosenPlayer();
	}
}

void Inventory::Render()
{
	if (!closed)
	{
		inventoryPanel.Render(engine->GetWindow());
		scrollbar.Render(engine->GetWindow());

		engine->GetWindow().draw(currentPlayerName);
		engine->GetWindow().draw(panelTitle);

		buttonNext.Render(*engine);
		buttonPrevious.Render(*engine);
		buttonClose.Render(*engine);

		int showTooltipForItem = -1;
		for (int i = currentUpperRow * 5; i < currentUpperRow * 5 + 25 && i < items.size(); i++)
		{
			items[i].second.Render(engine->GetWindow());

			if (items[i].second.GetGlobalRect().contains(engine->GetMousePos()))
			{
				showTooltipForItem = items[i].first.id;
			}
		}

		if (showTooltipForItem != -1)
		{
			tooltip.SetItemID(ItemID(showTooltipForItem));
			tooltip.ShowTooltip(engine->GetMousePos().x - 10, engine->GetMousePos().y - 10);
		}
	}
}


void Inventory::Open(int _player)
{
	closed = false;
	currentPlayer = _player;
	UpdateGUIForChosenPlayer();
}


void Inventory::OnItemAdded(Item _item)
{
	int xPos = (items.size() % 5) * 116 + inventoryPanel.GetGlobalRect().left + 832;
	int yPos = (items.size() / 5) * 116 + inventoryPanel.GetGlobalRect().top + 195;

	CSprite newSprite;
	newSprite.Load(g_pTextures->item[_item.id]);
	newSprite.SetPos(xPos, yPos);
	items.push_back(std::make_pair(_item, std::move(newSprite)));

	scrollbar.SetNumberOfSteps(std::max(1, (int)(items.size() - 21) / 5 + 1));
}