#include "SpeechBubble.h"
#include "Resources/TextureManager.hpp"
#include "Resources/FontManager.hpp"
#include "../Framework/Time.hpp"

SpeechBubble::SpeechBubble(sf::String &_text, Player *_player)
{
	player = _player;

    bubbleText.setFont(g_pFonts->f_showcard);
    bubbleText.setCharacterSize(18);
	bubbleText.setFillColor(sf::Color::Black);
    bubbleText.setString(_text);
	FitTextIntoMaxWidth();

	bubble.setTexture(g_pTextures->speechBubble);
	bubble.setSize(sf::Vector2f(bubbleText.getGlobalBounds().width + 20.0f, bubbleText.getGlobalBounds().height + 20.0f));
}

void SpeechBubble::Update()
{
	int x = player->GetRect().left + player->GetRect().width - 10;
	int y = player->GetRect().top - bubble.getGlobalBounds().height - 10;
	bubble.setPosition(x, y);

	int textX = bubble.getGlobalBounds().left + bubble.getGlobalBounds().width / 2 - bubbleText.getGlobalBounds().width / 2;
	int textY = bubble.getGlobalBounds().top + bubble.getGlobalBounds().height / 2 - bubbleText.getGlobalBounds().height / 2 - 17;
	bubbleText.setPosition(textX, textY);

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
			if (bubbleText.getGlobalBounds().width >= MAX_TEXT_WIDTH)
			{
				textString.insert(lastSpace, '\n');
				i++;
			}

			lastSpace = i;
		}
	}

	bubbleText.setString(textString);
}
