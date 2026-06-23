#include "SpeechBubble.h"
#include "Resources/TextureManager.hpp"
#include "Resources/FontManager.hpp"
#include "../engine/Time.hpp"

SpeechBubble::SpeechBubble(sf::String &_text, Player *_player)
{
	player = _player;

    bubbleText.setFont(g_pFonts->f_showcard);
    bubbleText.setCharacterSize(18);
	bubbleText.setFillColor(sf::Color::Black);
    bubbleText.setString(_text);
	FitTextIntoMaxWidth();

	bubble.setTexture(g_pTextures->speechBubble);
	bubble.setSize(sf::Vector2f(bubbleText.getGlobalBounds().size.x + 20.0f, bubbleText.getGlobalBounds().size.y + 20.0f));
}

void SpeechBubble::Update()
{
	int x = player->GetRect().position.x + player->GetRect().size.x - 10;
	int y = player->GetRect().position.y - bubble.getGlobalBounds().size.y - 10;
	bubble.setPosition(sf::Vector2f(x, y));

	int textX = bubble.getGlobalBounds().position.x + bubble.getGlobalBounds().size.x / 2 - bubbleText.getGlobalBounds().size.x / 2;
	int textY = bubble.getGlobalBounds().position.y + bubble.getGlobalBounds().size.y / 2 - bubbleText.getGlobalBounds().size.y / 2 - 17;
	bubbleText.setPosition(sf::Vector2f(textX, textY));

	duration -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
}

void SpeechBubble::Render(sf::RenderTarget &_target)
{
    _target.draw(bubble);
    _target.draw(bubbleText);
}

void SpeechBubble::SetDuration(float _duration)
{
	duration = _duration;
}

bool SpeechBubble::DurationExceeded() const
{
	return duration <= 0;
}


void SpeechBubble::FitTextIntoMaxWidth()
{
	sf::String textString = bubbleText.getString();
	
	int lastSpace = 0;

	for (int i = 0; i < textString.getSize(); i++)
	{
		if (textString[i] == ' ')
		{
			bubbleText.setString(textString.substring(0, i));
			if (bubbleText.getGlobalBounds().size.x >= MAX_TEXT_WIDTH)
			{
				textString.insert(lastSpace, '\n');
				i++;
			}

			lastSpace = i;
		}
	}

	bubbleText.setString(textString);
}
