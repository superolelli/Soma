#include "SkillPanel.hpp"
#include "Resources\StringManager.hpp"
#include "../Framework/Graphics/RichText.hpp"
#include "../Framework/Graphics/RoundedRectangleShape.hpp"
#include "Resources\ObjectPropertiesManager.hpp"


void SkillPanel::Init(GameStatus *_gameStatus, CGameEngine *_engine)
{
	gameStatus = _gameStatus;
	engine = _engine;

	currentPlayer = 0;
	currentAbility = 0;
	currentSkill = 0;

	closed = true;

	skillPanel.Load(g_pTextures->skillPanel);
	skillPanel.SetPos(150, 70);

	bridgePiece.Load(g_pTextures->skillPanelBridgePiece);
	bridgePiece.SetPos(skillPanel.GetGlobalRect().left + 525, skillPanel.GetGlobalRect().top + 176);

	skilledIndicator.Load(g_pTextures->skillPanelSkilledIndicator);

	abilityPlaceholders.Load(g_pTextures->skillPanelAbilityPlaceholder);
	abilityPlaceholders.SetPos(skillPanel.GetGlobalRect().left + 623, skillPanel.GetGlobalRect().top + 190);

	connectionsNotSkilled[0].Load(g_pTextures->skillPanelConnectionsNotSkilled[0]);
	connectionsNotSkilled[1].Load(g_pTextures->skillPanelConnectionsNotSkilled[1]);
	connectionsNotSkilled[2].Load(g_pTextures->skillPanelConnectionsNotSkilled[2]);
	connectionsNotSkilled[3].Load(g_pTextures->skillPanelConnectionsNotSkilled[3]);
	connectionsNotSkilled[4].Load(g_pTextures->skillPanelConnectionsNotSkilled[2]);
	connectionsNotSkilled[5].Load(g_pTextures->skillPanelConnectionsNotSkilled[2]);

	connectionsNotSkilled[0].SetPos(skillPanel.GetGlobalRect().left + 874, skillPanel.GetGlobalRect().top + 270);
	connectionsNotSkilled[1].SetPos(skillPanel.GetGlobalRect().left + 681, skillPanel.GetGlobalRect().top + 406);
	connectionsNotSkilled[2].SetPos(skillPanel.GetGlobalRect().left + 671, skillPanel.GetGlobalRect().top + 463);
	connectionsNotSkilled[3].SetPos(skillPanel.GetGlobalRect().left + 1030, skillPanel.GetGlobalRect().top + 410);
	connectionsNotSkilled[4].SetPos(skillPanel.GetGlobalRect().left + 1377, skillPanel.GetGlobalRect().top + 453);
	connectionsNotSkilled[5].SetPos(skillPanel.GetGlobalRect().left + 1036, skillPanel.GetGlobalRect().top + 534);

	connectionsSkilled[0].Load(g_pTextures->skillPanelConnectionsSkilled[0]);
	connectionsSkilled[1].Load(g_pTextures->skillPanelConnectionsSkilled[1]);
	connectionsSkilled[2].Load(g_pTextures->skillPanelConnectionsSkilled[2]);
	connectionsSkilled[3].Load(g_pTextures->skillPanelConnectionsSkilled[3]);
	connectionsSkilled[4].Load(g_pTextures->skillPanelConnectionsSkilled[2]);
	connectionsSkilled[5].Load(g_pTextures->skillPanelConnectionsSkilled[2]);

	connectionsSkilled[0].SetPos(skillPanel.GetGlobalRect().left + 874, skillPanel.GetGlobalRect().top + 270);
	connectionsSkilled[1].SetPos(skillPanel.GetGlobalRect().left + 681, skillPanel.GetGlobalRect().top + 406);
	connectionsSkilled[2].SetPos(skillPanel.GetGlobalRect().left + 671, skillPanel.GetGlobalRect().top + 463);
	connectionsSkilled[3].SetPos(skillPanel.GetGlobalRect().left + 1030, skillPanel.GetGlobalRect().top + 410);
	connectionsSkilled[4].SetPos(skillPanel.GetGlobalRect().left + 1377, skillPanel.GetGlobalRect().top + 453);
	connectionsSkilled[5].SetPos(skillPanel.GetGlobalRect().left + 1036, skillPanel.GetGlobalRect().top + 534);


	abilities[4][4];

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			abilities[i][j].Load(g_pTextures->abilities[i][j]);
			abilities[i][j].SetPos(skillPanel.GetGlobalRect().left + 398, skillPanel.GetGlobalRect().top + 195 + j * 155);
		}
	}


	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int a = 0; a < 8; a++)
			{
				skills[i][j][a].Load(g_pTextures->skills[i][j][a]);
				if(gameStatus->IsSkillAcquired(i, j, a) == false && SkillCanBeAcquired(i, j, a) == false)
					skills[i][j][a].SetColor(40, 40, 40);
			}

			skills[i][j][0].SetPos(abilityPlaceholders.GetGlobalRect().left + 233, abilityPlaceholders.GetGlobalRect().top + 180);
			skills[i][j][1].SetPos(abilityPlaceholders.GetGlobalRect().left + 12, abilityPlaceholders.GetGlobalRect().top + 182);
			skills[i][j][2].SetPos(abilityPlaceholders.GetGlobalRect().left + 11, abilityPlaceholders.GetGlobalRect().top + 468);
			skills[i][j][3].SetPos(abilityPlaceholders.GetGlobalRect().left + 517, abilityPlaceholders.GetGlobalRect().top + 179);
			skills[i][j][4].SetPos(abilityPlaceholders.GetGlobalRect().left + 718, abilityPlaceholders.GetGlobalRect().top + 181);
			skills[i][j][5].SetPos(abilityPlaceholders.GetGlobalRect().left + 716, abilityPlaceholders.GetGlobalRect().top + 468);
			skills[i][j][6].SetPos(abilityPlaceholders.GetGlobalRect().left + 378, abilityPlaceholders.GetGlobalRect().top + 330);
			skills[i][j][7].SetPos(abilityPlaceholders.GetGlobalRect().left + 378, abilityPlaceholders.GetGlobalRect().top + 467);
		}
	}


	currentSkillFrame.Load(g_pTextures->skillPanelSelectedSkillFrame);
	currentSkillFrame.SetPos(skills[currentPlayer][currentAbility][currentSkill].GetGlobalRect().left - 36, skills[currentPlayer][currentAbility][currentSkill].GetGlobalRect().top - 37);

	currentPlayerName.setCharacterSize(47);
	currentPlayerName.setFont(g_pFonts->f_blackwoodCastle);
	currentPlayerName.setFillColor(sf::Color::Black);
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(skillPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width)/2, skillPanel.GetGlobalRect().top + 55);

	panelTitle.setCharacterSize(70);
	panelTitle.setFont(g_pFonts->f_blackwoodCastle);
	panelTitle.setFillColor(sf::Color::Black);
	panelTitle.setString("Fähigkeiten");
	panelTitle.setPosition(skillPanel.GetGlobalRect().left + 850, skillPanel.GetGlobalRect().top + 70);

	for (int i = 0; i < 4; i++)
	{
		abilityName[i].setCharacterSize(24);
		abilityName[i].setFont(g_pFonts->f_showcard);
		abilityName[i].setFillColor(sf::Color::Black);
		abilityName[i].setString(g_pObjectProperties->playerAbilities[currentPlayer][i].name);
		abilityName[i].setPosition(skillPanel.GetGlobalRect().left + 224 - abilityName[i].getGlobalBounds().width / 2, skillPanel.GetGlobalRect().top + 235 + i * 155);
	}

	chosenSkillName.setCharacterSize(25);
	chosenSkillName.setFont(g_pFonts->f_showcard);
	chosenSkillName.setFillColor(sf::Color::Black);
	chosenSkillName.setString(g_pObjectProperties->skills[currentPlayer][currentAbility][currentSkill].name);
	chosenSkillName.setPosition(skillPanel.GetGlobalRect().left + 700 - chosenSkillName.getGlobalBounds().width / 2, skillPanel.GetGlobalRect().top + 810);

	buttonNext.Load(g_pTextures->skillPanelButtonNext, Buttontypes::Up);
	buttonNext.SetPos(skillPanel.GetGlobalRect().left + 217, skillPanel.GetGlobalRect().top + 66);

	buttonPrevious.Load(g_pTextures->skillPanelButtonPrevious, Buttontypes::Up);
	buttonPrevious.SetPos(skillPanel.GetGlobalRect().left + 15, skillPanel.GetGlobalRect().top + 66);

	buttonClose.Load(g_pTextures->skillPanelButtonClose, Buttontypes::Motion_Up);
	buttonClose.SetPos(skillPanel.GetGlobalRect().left + 1475, skillPanel.GetGlobalRect().top + 66);

	buttonBuy.Load(g_pTextures->bangGenericButton, Buttontypes::Motion_Up);
	buttonBuy.SetButtonstring("Kaufen");
	buttonBuy.SetButtontextFont(g_pFonts->f_trajan);
	buttonBuy.SetButtontextCharactersize(30);
	buttonBuy.SetPos(skillPanel.GetGlobalRect().left + 985, skillPanel.GetGlobalRect().top + 789);

	abilityPanelRect[0].left = skillPanel.GetGlobalRect().left + 51;
	abilityPanelRect[0].top = skillPanel.GetGlobalRect().top + 176;
	abilityPanelRect[0].width = 517;
	abilityPanelRect[0].height = 145;

	abilityPanelRect[1].left = skillPanel.GetGlobalRect().left + 51;
	abilityPanelRect[1].top = skillPanel.GetGlobalRect().top + 328;
	abilityPanelRect[1].width = 517;
	abilityPanelRect[1].height = 145;

	abilityPanelRect[2].left = skillPanel.GetGlobalRect().left + 51;
	abilityPanelRect[2].top = skillPanel.GetGlobalRect().top + 483;
	abilityPanelRect[2].width = 517;
	abilityPanelRect[2].height = 145;

	abilityPanelRect[3].left = skillPanel.GetGlobalRect().left + 51;
	abilityPanelRect[3].top = skillPanel.GetGlobalRect().top + 636;
	abilityPanelRect[3].width = 517;
	abilityPanelRect[3].height = 145;
}


void SkillPanel::Open()
{
	closed = false;
}

void SkillPanel::Update()
{
	if (!closed)
	{
		int lastCurrentPlayer = currentPlayer;

		if (buttonClose.Update(*engine) == true)
			closed = true;

		if (buttonNext.Update(*engine) == true)
			currentPlayer++;

		if (buttonPrevious.Update(*engine) == true)
			currentPlayer--;

		if (buttonBuy.Update(*engine) == true && SkillCanBeAcquired(currentPlayer, currentAbility, currentSkill))
		{
			if (gameStatus->GetDiceAmount() >= skillCost[currentSkill])
			{
				gameStatus->RemoveDice(skillCost[currentSkill]);
				gameStatus->AcquireSkill(currentPlayer, currentAbility, currentSkill);
				RecolorSkills();
			}
		}

		if (currentPlayer > 3)
			currentPlayer = 0;
		if (currentPlayer < 0)
			currentPlayer = 3;

		if (currentPlayer != lastCurrentPlayer)
		{
			currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
			currentPlayerName.setPosition(skillPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, skillPanel.GetGlobalRect().top + 55);

			UpdateAbilityNames();
			UpdateChosenSkillName();
		}


		for (int i = 0; i < 4; i++)
		{
			if (abilityPanelRect[i].contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
			{
				currentAbility = i;
				bridgePiece.SetPos(skillPanel.GetGlobalRect().left + 525, abilityPanelRect[i].top);
				UpdateChosenSkillName();
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (skills[currentPlayer][currentAbility][i].GetGlobalRect().contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
			{
				currentSkill = i;
				currentSkillFrame.SetPos(skills[currentPlayer][currentAbility][currentSkill].GetGlobalRect().left - 36, skills[currentPlayer][currentAbility][currentSkill].GetGlobalRect().top - 37);
				UpdateChosenSkillName();
			}
		}
	}
}


void SkillPanel::UpdateChosenSkillName()
{
	chosenSkillName.setString(g_pObjectProperties->skills[currentPlayer][currentAbility][currentSkill].name);
	chosenSkillName.setPosition(skillPanel.GetGlobalRect().left + 700 - chosenSkillName.getGlobalBounds().width / 2, skillPanel.GetGlobalRect().top + 810);
}


void SkillPanel::UpdateAbilityNames()
{
	for (int i = 0; i < 4; i++)
	{
		abilityName[i].setString(g_pObjectProperties->playerAbilities[currentPlayer][i].name);

		if (abilityName[i].getGlobalBounds().width >= 346)
		{
			sf::String newString = abilityName[i].getString();
			newString.insert(abilityName[i].getString().find(" ") + 1, "\n");
			abilityName[i].setString(newString);
		}

		abilityName[i].setPosition(skillPanel.GetGlobalRect().left + 224 - abilityName[i].getGlobalBounds().width / 2, skillPanel.GetGlobalRect().top + 235 + i * 155);
	}
}

void SkillPanel::Render()
{
	if (!closed)
	{
		skillPanel.Render(engine->GetWindow());
		bridgePiece.Render(engine->GetWindow());

		connectionsSkilled[0].Render(engine->GetWindow());

		RenderConnection(1, 0);
		RenderConnection(2, 1);
		RenderConnection(3, 3);
		RenderConnection(4, 4);
		RenderConnection(5, 6);

		abilityPlaceholders.Render(engine->GetWindow());

		RenderSkilledIndicators();

		for (auto &a : abilities[currentPlayer])
			a.Render(engine->GetWindow());

		auto xPos = abilities[currentPlayer][currentAbility].GetGlobalRect().left;
		auto yPos = abilities[currentPlayer][currentAbility].GetGlobalRect().top;
		abilities[currentPlayer][currentAbility].SetPos(abilityPlaceholders.GetGlobalRect().left + 378, abilityPlaceholders.GetGlobalRect().top + 12);
		abilities[currentPlayer][currentAbility].Render(engine->GetWindow());
		abilities[currentPlayer][currentAbility].SetPos(xPos, yPos);
		

		for (auto &s : skills[currentPlayer][currentAbility])
			s.Render(engine->GetWindow());

		currentSkillFrame.Render(engine->GetWindow());

		engine->GetWindow().draw(currentPlayerName);
		engine->GetWindow().draw(panelTitle);
		engine->GetWindow().draw(chosenSkillName);

		for(auto &n : abilityName)
			engine->GetWindow().draw(n);

		buttonNext.Render(*engine);
		buttonPrevious.Render(*engine);
		buttonClose.Render(*engine);
		buttonBuy.Render(*engine);

		for (int i = 0; i < 8; i++)
		{
			if (skills[currentPlayer][currentAbility][i].GetGlobalRect().contains(engine->GetMousePos()))
				ShowTooltip(i);
		}
	}
}



void SkillPanel::RenderConnection(int connection, int parentSkill)
{
	if (gameStatus->IsSkillAcquired(currentPlayer, currentAbility, parentSkill))
		connectionsSkilled[connection].Render(engine->GetWindow());
	else
		connectionsNotSkilled[connection].Render(engine->GetWindow());
}



void SkillPanel::RenderSkilledIndicators()
{
	skilledIndicator.SetPos(abilityPlaceholders.GetGlobalRect().left + 368, abilityPlaceholders.GetGlobalRect().top + 3);
	skilledIndicator.Render(engine->GetWindow());

	for (int i = 0; i < 8; i++)
	{
		if (gameStatus->IsSkillAcquired(currentPlayer, currentAbility, i))
		{
			skilledIndicator.SetPos(skills[currentPlayer][currentAbility][i].GetGlobalRect().left - 9, skills[currentPlayer][currentAbility][i].GetGlobalRect().top - 9);
			skilledIndicator.Render(engine->GetWindow());
		}
	}
}


void SkillPanel::ShowTooltip(int _skill)
{
	sfe::RichText tooltip;
	tooltip.setCharacterSize(18);
	tooltip.setFont(g_pFonts->f_arial);
	tooltip.setString(g_pObjectProperties->skills[currentPlayer][currentAbility][_skill].description);
	tooltip.setPosition(engine->GetMousePos().x + 10, engine->GetMousePos().y - (tooltip.getGlobalBounds().height + 15));

	sf::FloatRect backgroundRect = tooltip.getLocalBounds();
	sf::RoundedRectangleShape background(sf::Vector2f(backgroundRect.width + 20.0f, backgroundRect.height + 20.0f), 8, 20);
	background.setFillColor(sf::Color(0, 0, 0, 220));
	background.setOutlineThickness(2.0f);
	background.setOutlineColor(sf::Color(40, 40, 40));
	background.setPosition(tooltip.getPosition() + sf::Vector2f(-10.0f, -7.0f));

	engine->GetWindow().draw(background);
	engine->GetWindow().draw(tooltip);
}


bool SkillPanel::SkillCanBeAcquired(int _player, int _ability, int _skill)
{
	if (gameStatus->IsSkillAcquired(_player, _ability, _skill))
		return false;

	if (_skill == 0 || _skill == 3)
		return true;

	switch (_skill)
	{
	case 1:
		return gameStatus->IsSkillAcquired(_player, _ability, 0);
		break;
	case 2: 
		return gameStatus->IsSkillAcquired(_player, _ability, 1);
		break;
	case 4:
		return gameStatus->IsSkillAcquired(_player, _ability, 3);
	case 5:
		return gameStatus->IsSkillAcquired(_player, _ability, 4);
	case 6:
		return gameStatus->IsSkillAcquired(_player, _ability, 0)
			   && gameStatus->IsSkillAcquired(_player, _ability, 3);
		break;
	case 7:
		return gameStatus->IsSkillAcquired(_player, _ability, 6);
		break;
	default:
		return false;
	}
}


void SkillPanel::RecolorSkills()
{
	for (int i = 0; i < 8; i++)
	{
		if (gameStatus->IsSkillAcquired(currentPlayer, currentAbility, i) == false && SkillCanBeAcquired(currentPlayer, currentAbility, i) == false)
			skills[currentPlayer][currentAbility][i].SetColor(40, 40, 40);
		else
			skills[currentPlayer][currentAbility][i].SetColor(255, 255, 255);
	}
}