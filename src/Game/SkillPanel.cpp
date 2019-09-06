#include "SkillPanel.hpp"
#include "Resources\StringManager.hpp"

void SkillPanel::Init(GameStatus *_gameStatus, CGameEngine *_engine)
{
	gameStatus = _gameStatus;
	engine = _engine;

	currentPlayer = 0;
	currentAbility = 0;

	closed = true;

	skillPanel.Load(g_pTextures->skillPanel);
	skillPanel.SetPos(100, 100);

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
			abilities[i][j].SetPos(skillPanel.GetGlobalRect().left + 400, skillPanel.GetGlobalRect().top + 195 + j * 155);
		}
	}


	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int a = 0; a < 8; a++)
			{
				skills[i][j][a].Load(g_pTextures->skills[i][j][a]);

				if(gameStatus->IsSkillAcquired(i, j, a) == false)
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

	currentPlayerName.setCharacterSize(47);
	currentPlayerName.setFont(g_pFonts->f_blackwoodCastle);
	currentPlayerName.setFillColor(sf::Color::Black);
	currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
	currentPlayerName.setPosition(skillPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width)/2, skillPanel.GetGlobalRect().top + 55);

	buttonNext.Load(g_pTextures->skillPanelButtonNext, Buttontypes::Up);
	buttonNext.SetPos(skillPanel.GetGlobalRect().left + 217, skillPanel.GetGlobalRect().top + 66);

	buttonPrevious.Load(g_pTextures->skillPanelButtonPrevious, Buttontypes::Up);
	buttonPrevious.SetPos(skillPanel.GetGlobalRect().left + 15, skillPanel.GetGlobalRect().top + 66);

	buttonClose.Load(g_pTextures->skillPanelButtonClose, Buttontypes::Motion_Up);
	buttonClose.SetPos(skillPanel.GetGlobalRect().left + 1475, skillPanel.GetGlobalRect().top + 66);

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
		if (buttonClose.Update(*engine) == true)
			closed = true;

		if (buttonNext.Update(*engine) == true)
			currentPlayer++;

		if (buttonPrevious.Update(*engine) == true)
			currentPlayer--;

		if (currentPlayer > 3)
			currentPlayer = 0;
		if (currentPlayer < 0)
			currentPlayer = 3;

		currentPlayerName.setString(g_pStringContainer->combatantNames[currentPlayer]);
		currentPlayerName.setPosition(skillPanel.GetGlobalRect().left + 52 + (162 - currentPlayerName.getLocalBounds().width) / 2, skillPanel.GetGlobalRect().top + 55);

		for (int i = 0; i < 4; i++)
		{
			if (abilityPanelRect[i].contains(engine->GetMousePos()) && engine->GetButtonstates(ButtonID::Left) == Keystates::Pressed)
			{
				currentAbility = i;
				bridgePiece.SetPos(skillPanel.GetGlobalRect().left + 525, abilityPanelRect[i].top);
			}
		}
	}
}



void SkillPanel::Render()
{
	if (!closed)
	{
		skillPanel.Render(engine->GetWindow());
		bridgePiece.Render(engine->GetWindow());

		connectionsSkilled[0].Render(engine->GetWindow());

		RenderConnection(1, 1);
		RenderConnection(2, 2);
		RenderConnection(3, 4);
		RenderConnection(4, 5);
		RenderConnection(5, 7);

		abilityPlaceholders.Render(engine->GetWindow());

		for (auto &a : abilities[currentPlayer])
			a.Render(engine->GetWindow());

		auto xPos = abilities[currentPlayer][currentAbility].GetGlobalRect().left;
		auto yPos = abilities[currentPlayer][currentAbility].GetGlobalRect().top;
		abilities[currentPlayer][currentAbility].SetPos(abilityPlaceholders.GetGlobalRect().left + 378, abilityPlaceholders.GetGlobalRect().top + 12);
		abilities[currentPlayer][currentAbility].Render(engine->GetWindow());
		abilities[currentPlayer][currentAbility].SetPos(xPos, yPos);
		

		for (auto &s : skills[currentPlayer][currentAbility])
			s.Render(engine->GetWindow());

		engine->GetWindow().draw(currentPlayerName);

		buttonNext.Render(*engine);
		buttonPrevious.Render(*engine);
		buttonClose.Render(*engine);
	}
}


void SkillPanel::RenderConnection(int connection, int parentAbility)
{
	if (gameStatus->IsSkillAcquired(currentPlayer, currentAbility, parentAbility))
		connectionsSkilled[connection].Render(engine->GetWindow());
	else
		connectionsNotSkilled[connection].Render(engine->GetWindow());
}