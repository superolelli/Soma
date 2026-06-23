#include "SkillPanel.hpp"
#include "Resources\StringManager.hpp"
#include "../engine/Graphics/RichText.hpp"
#include "../engine/Graphics/RoundedRectangleShape.hpp"
#include "Resources\ObjectPropertiesManager.hpp"
#include "Resources\SoundManager.hpp"


SkillPanel::SkillPanel(CGameEngine *_engine)
	: engine(_engine)
	, currentPlayer(0)
	, currentAbility(0)
	, currentSkill(0)
	, closed(true)
	, skillPanel(g_pTextures->skillPanel)
	, bridgePiece(g_pTextures->skillPanelBridgePiece)
	, skilledIndicator(g_pTextures->skillPanelSkilledIndicator)
	, abilityPlaceholders(g_pTextures->skillPanelAbilityPlaceholder)
	, connectionsNotSkilled{ 
		  {g_pTextures->skillPanelConnectionsNotSkilled[0]}
		, {g_pTextures->skillPanelConnectionsNotSkilled[1]}
		, {g_pTextures->skillPanelConnectionsNotSkilled[2]}
		, {g_pTextures->skillPanelConnectionsNotSkilled[3]}
		, {g_pTextures->skillPanelConnectionsNotSkilled[2]}
		, {g_pTextures->skillPanelConnectionsNotSkilled[2]}}
	, connectionsSkilled{
		  {g_pTextures->skillPanelConnectionsSkilled[0]}
		, {g_pTextures->skillPanelConnectionsSkilled[1]}
		, {g_pTextures->skillPanelConnectionsSkilled[2]}
		, {g_pTextures->skillPanelConnectionsSkilled[3]}
		, {g_pTextures->skillPanelConnectionsSkilled[2]}
		, {g_pTextures->skillPanelConnectionsSkilled[2]} }
	, currentSkillFrame(g_pTextures->skillPanelSelectedSkillFrame)
	, diceSymbol(g_pTextures->skillPanelDice)
	, buttonNext(g_pTextures->skillPanelButtonNext, Buttontypes::Up)
	, buttonPrevious(g_pTextures->skillPanelButtonPrevious, Buttontypes::Up)
	, buttonClose(g_pTextures->skillPanelButtonClose, Buttontypes::Motion_Up)
	, buttonBuy(g_pTextures->bangGenericButton, Buttontypes::Motion_Up)
{
	skillPanel.SetPos(150, 70);
	bridgePiece.SetPos(skillPanel.GetGlobalRect().position.x + 525, skillPanel.GetGlobalRect().position.y + 176);
	abilityPlaceholders.SetPos(skillPanel.GetGlobalRect().position.x + 623, skillPanel.GetGlobalRect().position.y + 190);

	connectionsNotSkilled[0].SetPos(skillPanel.GetGlobalRect().position.x + 874, skillPanel.GetGlobalRect().position.y + 270);
	connectionsNotSkilled[1].SetPos(skillPanel.GetGlobalRect().position.x + 681, skillPanel.GetGlobalRect().position.y + 406);
	connectionsNotSkilled[2].SetPos(skillPanel.GetGlobalRect().position.x + 671, skillPanel.GetGlobalRect().position.y + 463);
	connectionsNotSkilled[3].SetPos(skillPanel.GetGlobalRect().position.x + 1030, skillPanel.GetGlobalRect().position.y + 410);
	connectionsNotSkilled[4].SetPos(skillPanel.GetGlobalRect().position.x + 1377, skillPanel.GetGlobalRect().position.y + 453);
	connectionsNotSkilled[5].SetPos(skillPanel.GetGlobalRect().position.x + 1036, skillPanel.GetGlobalRect().position.y + 534);

	connectionsSkilled[0].SetPos(skillPanel.GetGlobalRect().position.x + 874, skillPanel.GetGlobalRect().position.y + 270);
	connectionsSkilled[1].SetPos(skillPanel.GetGlobalRect().position.x + 681, skillPanel.GetGlobalRect().position.y + 406);
	connectionsSkilled[2].SetPos(skillPanel.GetGlobalRect().position.x + 671, skillPanel.GetGlobalRect().position.y + 463);
	connectionsSkilled[3].SetPos(skillPanel.GetGlobalRect().position.x + 1030, skillPanel.GetGlobalRect().position.y + 410);
	connectionsSkilled[4].SetPos(skillPanel.GetGlobalRect().position.x + 1377, skillPanel.GetGlobalRect().position.y + 453);
	connectionsSkilled[5].SetPos(skillPanel.GetGlobalRect().position.x + 1036, skillPanel.GetGlobalRect().position.y + 534);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			abilities[i][j] = new CSprite(g_pTextures->abilities[i][j]);
			abilities[i][j]->SetPos(skillPanel.GetGlobalRect().position.x + 398, skillPanel.GetGlobalRect().position.y + 195 + j * 155);
		}
	}


	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int a = 0; a < 8; a++)
			{
				skills[i][j][a] = new CSprite(g_pTextures->skills[i][j][a]);
				if(g_pGameStatus->IsSkillAcquired(i, j, a) == false && SkillCanBeAcquired(i, j, a) == false)
					skills[i][j][a]->SetColor(40, 40, 40);
			}

			skills[i][j][0]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 233, abilityPlaceholders.GetGlobalRect().position.y + 180);
			skills[i][j][1]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 12, abilityPlaceholders.GetGlobalRect().position.y + 182);
			skills[i][j][2]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 11, abilityPlaceholders.GetGlobalRect().position.y + 468);
			skills[i][j][3]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 517, abilityPlaceholders.GetGlobalRect().position.y + 179);
			skills[i][j][4]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 718, abilityPlaceholders.GetGlobalRect().position.y + 181);
			skills[i][j][5]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 716, abilityPlaceholders.GetGlobalRect().position.y + 468);
			skills[i][j][6]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 378, abilityPlaceholders.GetGlobalRect().position.y + 330);
			skills[i][j][7]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 378, abilityPlaceholders.GetGlobalRect().position.y + 467);
		}
	}

	currentSkillFrame.SetPos(skills[currentPlayer][currentAbility][currentSkill]->GetGlobalRect().position.x - 36, skills[currentPlayer][currentAbility][currentSkill]->GetGlobalRect().position.y - 37);
	diceSymbol.SetPos(skillPanel.GetGlobalRect().position.x + 1240, skillPanel.GetGlobalRect().position.y + 795);

	currentPlayerName.setCharacterSize(47);
	currentPlayerName.setFont(g_pFonts->f_blackwoodCastle);
	currentPlayerName.setFillColor(sf::Color::Black);
	currentPlayerName.setString(g_pObjectProperties->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(sf::Vector2f(skillPanel.GetGlobalRect().position.x + 52 + (162 - currentPlayerName.getLocalBounds().size.x)/2, skillPanel.GetGlobalRect().position.y + 55));

	panelTitle.setCharacterSize(70);
	panelTitle.setFont(g_pFonts->f_blackwoodCastle);
	panelTitle.setFillColor(sf::Color::Black);
	panelTitle.setString("F�higkeiten");
	panelTitle.setPosition(sf::Vector2f(skillPanel.GetGlobalRect().position.x + 850, skillPanel.GetGlobalRect().position.y + 70));

	for (int i = 0; i < 4; i++)
	{
		abilityName[i].setCharacterSize(24);
		abilityName[i].setFont(g_pFonts->f_showcard);
		abilityName[i].setFillColor(sf::Color::Black);
		abilityName[i].setString(g_pObjectProperties->playerAbilities[currentPlayer][i].name);
		abilityName[i].setPosition(sf::Vector2f(skillPanel.GetGlobalRect().position.x + 224 - abilityName[i].getGlobalBounds().size.x / 2, skillPanel.GetGlobalRect().position.y + 235 + i * 155));
	}

	chosenSkillName.setCharacterSize(25);
	chosenSkillName.setFont(g_pFonts->f_showcard);
	chosenSkillName.setFillColor(sf::Color::Black);
	chosenSkillName.setString(g_pObjectProperties->skills[currentPlayer][currentAbility][currentSkill].name);
	chosenSkillName.setPosition(sf::Vector2f(skillPanel.GetGlobalRect().position.x + 700 - chosenSkillName.getGlobalBounds().size.x / 2, skillPanel.GetGlobalRect().position.y + 810));

	chosenSkillPrice.setCharacterSize(26);
	chosenSkillPrice.setFont(g_pFonts->f_kingArthur);
	chosenSkillPrice.setFillColor(sf::Color::White);
	chosenSkillPrice.setOutlineColor(sf::Color::Black);
	chosenSkillPrice.setOutlineThickness(3);
	chosenSkillPrice.setString(std::to_string(skillCost[currentSkill]));
	chosenSkillPrice.setPosition(sf::Vector2f(diceSymbol.GetGlobalRect().position.x + 65, diceSymbol.GetGlobalRect().position.y + 20));

	buttonNext.SetPos(skillPanel.GetGlobalRect().position.x + 217, skillPanel.GetGlobalRect().position.y + 66);
	buttonNext.SetCallback([](){g_pSounds->PlaySound(soundID::CLICK);});

	buttonPrevious.SetPos(skillPanel.GetGlobalRect().position.x + 15, skillPanel.GetGlobalRect().position.y + 66);
	buttonPrevious.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonClose.SetPos(skillPanel.GetGlobalRect().position.x + 1475, skillPanel.GetGlobalRect().position.y + 66);
	buttonClose.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	buttonBuy.SetButtonstring("Kaufen");
	buttonBuy.SetButtontextFont(g_pFonts->f_trajan);
	buttonBuy.SetButtontextCharactersize(30);
	buttonBuy.SetPos(skillPanel.GetGlobalRect().position.x + 984, skillPanel.GetGlobalRect().position.y + 789);
	buttonBuy.SetCallback([]() {g_pSounds->PlaySound(soundID::CLICK); });

	abilityPanelRect[0].position.x = skillPanel.GetGlobalRect().position.x + 51;
	abilityPanelRect[0].position.y = skillPanel.GetGlobalRect().position.y + 176;
	abilityPanelRect[0].size.x = 517;
	abilityPanelRect[0].size.y = 145;

	abilityPanelRect[1].position.x = skillPanel.GetGlobalRect().position.x + 51;
	abilityPanelRect[1].position.y = skillPanel.GetGlobalRect().position.y + 328;
	abilityPanelRect[1].size.x = 517;
	abilityPanelRect[1].size.y = 145;

	abilityPanelRect[2].position.x = skillPanel.GetGlobalRect().position.x + 51;
	abilityPanelRect[2].position.y = skillPanel.GetGlobalRect().position.y + 483;
	abilityPanelRect[2].size.x = 517;
	abilityPanelRect[2].size.y = 145;

	abilityPanelRect[3].position.x = skillPanel.GetGlobalRect().position.x + 51;
	abilityPanelRect[3].position.y = skillPanel.GetGlobalRect().position.y + 636;
	abilityPanelRect[3].size.x = 517;
	abilityPanelRect[3].size.y = 145;
}


void SkillPanel::Open(int _player)
{
	closed = false;
	currentPlayer = _player;
	UpdateGUIForChosenPlayer();
}


void SkillPanel::Update()
{
	if (!closed)
	{
		if (buttonClose.Update(*engine) == true)
			closed = true;

		CheckButtonsForPlayerChoosing();
		CheckBuyButton();

		for (int i = 0; i < 4; i++)
		{
			if (abilityPanelRect[i].contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
			{
				currentAbility = i;
				bridgePiece.SetPos(skillPanel.GetGlobalRect().position.x + 525, abilityPanelRect[i].position.y);
				UpdateGUIForChosenSkill();
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (skills[currentPlayer][currentAbility][i]->GetGlobalRect().contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
			{
				currentSkill = i;
				currentSkillFrame.SetPos(skills[currentPlayer][currentAbility][currentSkill]->GetGlobalRect().position.x - 36, skills[currentPlayer][currentAbility][currentSkill]->GetGlobalRect().position.y - 37);
				UpdateGUIForChosenSkill();
			}
		}
	}
}

void SkillPanel::UpdateGUIForChosenSkill()
{
	UpdateChosenSkillName();
	chosenSkillPrice.setString(std::to_string(skillCost[currentSkill]));
	UpdateCurrentSkillFrame();
	UpdateBuyButton();
}

void SkillPanel::UpdateGUIForChosenPlayer()
{
	currentPlayerName.setString(g_pObjectProperties->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(sf::Vector2f(skillPanel.GetGlobalRect().position.x + 52 + (162 - currentPlayerName.getLocalBounds().size.x) / 2, skillPanel.GetGlobalRect().position.y + 55));

	UpdateAbilityNames();
	UpdateGUIForChosenSkill();
}


void SkillPanel::CheckButtonsForPlayerChoosing()
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


void SkillPanel::CheckBuyButton()
{
	if (buttonBuy.Update(*engine) == true && SkillCanBeAcquired(currentPlayer, currentAbility, currentSkill))
	{
		if (g_pGameStatus->GetDiceAmount() >= skillCost[currentSkill])
		{
			g_pGameStatus->RemoveDice(skillCost[currentSkill]);
			g_pGameStatus->AcquireSkill(currentPlayer, currentAbility, currentSkill);
			g_pSounds->PlaySound(soundID::SKILL_ACQUIRED);
			RecolorSkills();
			UpdateCurrentSkillFrame();
		}
	}
}



void SkillPanel::UpdateBuyButton()
{
	if (SkillCanBeAcquired(currentPlayer, currentAbility, currentSkill) && skillCost[currentSkill] <= g_pGameStatus->GetDiceAmount())
		buttonBuy.SetEnabled();
	else
		buttonBuy.SetDisabled();
}


void SkillPanel::UpdateChosenSkillName()
{
	chosenSkillName.setString(g_pObjectProperties->skills[currentPlayer][currentAbility][currentSkill].name);
	chosenSkillName.setPosition(sf::Vector2f(skillPanel.GetGlobalRect().position.x + 700 - chosenSkillName.getGlobalBounds().size.x / 2, skillPanel.GetGlobalRect().position.y + 810));
}


void SkillPanel::UpdateCurrentSkillFrame()
{
	if (!SkillCanBeAcquired(currentPlayer, currentAbility, currentSkill) &&
		!g_pGameStatus->IsSkillAcquired(currentPlayer, currentAbility, currentSkill))
	{
		currentSkillFrame.SetColor(180, 150, 150);
	}
	else
		currentSkillFrame.SetColor(255, 255, 255);
}


void SkillPanel::UpdateAbilityNames()
{
	for (int i = 0; i < 4; i++)
	{
		abilityName[i].setString(g_pObjectProperties->playerAbilities[currentPlayer][i].name);

		if (abilityName[i].getGlobalBounds().size.x >= 346)
		{
			sf::String newString = abilityName[i].getString();
			newString.insert(abilityName[i].getString().find(" ") + 1, "\n");
			abilityName[i].setString(newString);
		}

		abilityName[i].setPosition(sf::Vector2f(skillPanel.GetGlobalRect().position.x + 224 - abilityName[i].getGlobalBounds().size.x / 2, skillPanel.GetGlobalRect().position.y + 235 + i * 155));
	}
}

void SkillPanel::Render()
{
	if (!closed)
	{
		skillPanel.Render(engine->GetRenderTarget());
		bridgePiece.Render(engine->GetRenderTarget());

		connectionsSkilled[0].Render(engine->GetRenderTarget());

		RenderConnection(1, 0);
		RenderConnection(2, 1);
		RenderConnection(3, 3);
		RenderConnection(4, 4);
		RenderConnection(5, 6);

		abilityPlaceholders.Render(engine->GetRenderTarget());

		RenderSkilledIndicators();

		for (auto &a : abilities[currentPlayer])
			a->Render(engine->GetRenderTarget());

		auto xPos = abilities[currentPlayer][currentAbility]->GetGlobalRect().position.x;
		auto yPos = abilities[currentPlayer][currentAbility]->GetGlobalRect().position.y;
		abilities[currentPlayer][currentAbility]->SetPos(abilityPlaceholders.GetGlobalRect().position.x + 378, abilityPlaceholders.GetGlobalRect().position.y + 12);
		abilities[currentPlayer][currentAbility]->Render(engine->GetRenderTarget());

		auto abilityRect = abilities[currentPlayer][currentAbility]->GetGlobalRect();
		abilities[currentPlayer][currentAbility]->SetPos(xPos, yPos);
		

		for (auto &s : skills[currentPlayer][currentAbility])
			s->Render(engine->GetRenderTarget());

		currentSkillFrame.Render(engine->GetRenderTarget());

		engine->GetRenderTarget().draw(currentPlayerName);
		engine->GetRenderTarget().draw(panelTitle);
		engine->GetRenderTarget().draw(chosenSkillName);

		for(auto &n : abilityName)
			engine->GetRenderTarget().draw(n);

		if (!g_pGameStatus->IsSkillAcquired(currentPlayer, currentAbility, currentSkill))
		{
			diceSymbol.Render(engine->GetRenderTarget());
			engine->GetRenderTarget().draw(chosenSkillPrice);
			buttonBuy.Render(engine->GetRenderTarget());
		}

		buttonNext.Render(engine->GetRenderTarget());
		buttonPrevious.Render(engine->GetRenderTarget());
		buttonClose.Render(engine->GetRenderTarget());

		for (int i = 0; i < 8; i++)
		{
			if (skills[currentPlayer][currentAbility][i]->GetGlobalRect().contains(engine->GetMousePos()))
				ShowTooltip(i);
		}

		if (abilityRect.contains(engine->GetMousePos()))
			ShowAbilityTooltip(abilityRect);
	}
}



void SkillPanel::RenderConnection(int connection, int parentSkill)
{
	if (g_pGameStatus->IsSkillAcquired(currentPlayer, currentAbility, parentSkill))
		connectionsSkilled[connection].Render(engine->GetRenderTarget());
	else
		connectionsNotSkilled[connection].Render(engine->GetRenderTarget());
}



void SkillPanel::RenderSkilledIndicators()
{
	skilledIndicator.SetPos(abilityPlaceholders.GetGlobalRect().position.x + 368, abilityPlaceholders.GetGlobalRect().position.y + 3);
	skilledIndicator.Render(engine->GetRenderTarget());

	for (int i = 0; i < 8; i++)
	{
		if (g_pGameStatus->IsSkillAcquired(currentPlayer, currentAbility, i))
		{
			skilledIndicator.SetPos(skills[currentPlayer][currentAbility][i]->GetGlobalRect().position.x - 9, skills[currentPlayer][currentAbility][i]->GetGlobalRect().position.y - 9);
			skilledIndicator.Render(engine->GetRenderTarget());
		}
	}
}


void SkillPanel::ShowTooltip(int _skill)
{
	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(g_pObjectProperties->skills[currentPlayer][currentAbility][_skill].description);
	tooltip.setPosition(sf::Vector2f(engine->GetMousePos().x + 10, engine->GetMousePos().y - (tooltip.getGlobalBounds().size.y + 15)));

	sf::FloatRect backgroundRect = tooltip.getLocalBounds();
	sf::RoundedRectangleShape background(sf::Vector2f(backgroundRect.size.x + 20.0f, backgroundRect.size.y + 20.0f), 8, 20);
	background.setFillColor(sf::Color(0, 0, 0, 220));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(tooltip.getPosition() + sf::Vector2f(-10.0f, -7.0f));

	engine->GetRenderTarget().draw(background);
	engine->GetRenderTarget().draw(tooltip);
}



void SkillPanel::ShowAbilityTooltip(sf::IntRect &_abilityRect)
{
	abilityTooltip.SetAbilityID(currentAbility);
	abilityTooltip.SetPlayerID(currentPlayer);
	abilityTooltip.ShowTooltip(engine->GetRenderTarget(), _abilityRect.position.x + 135, _abilityRect.position.y - 10);
	abilityTooltip.ShowPossibleTargets(engine->GetRenderTarget(), _abilityRect.position.x - 305, _abilityRect.position.y - 10, true);
}

bool SkillPanel::SkillCanBeAcquired(int _player, int _ability, int _skill)
{
	if (g_pGameStatus->IsSkillAcquired(_player, _ability, _skill))
		return false;

	if (_skill == 0 || _skill == 3)
		return true;

	switch (_skill)
	{
	case 1:
		return g_pGameStatus->IsSkillAcquired(_player, _ability, 0);
		break;
	case 2: 
		return g_pGameStatus->IsSkillAcquired(_player, _ability, 1);
		break;
	case 4:
		return g_pGameStatus->IsSkillAcquired(_player, _ability, 3);
	case 5:
		return g_pGameStatus->IsSkillAcquired(_player, _ability, 4);
	case 6:
		return g_pGameStatus->IsSkillAcquired(_player, _ability, 0)
			   && g_pGameStatus->IsSkillAcquired(_player, _ability, 3);
		break;
	case 7:
		return g_pGameStatus->IsSkillAcquired(_player, _ability, 6);
		break;
	default:
		return false;
	}
}


void SkillPanel::RecolorSkills()
{
	for (int i = 0; i < 8; i++)
	{
		if (g_pGameStatus->IsSkillAcquired(currentPlayer, currentAbility, i) == false && SkillCanBeAcquired(currentPlayer, currentAbility, i) == false)
			skills[currentPlayer][currentAbility][i]->SetColor(40, 40, 40);
		else
			skills[currentPlayer][currentAbility][i]->SetColor(255, 255, 255);
	}
}