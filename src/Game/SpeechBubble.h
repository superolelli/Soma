#pragma once

#include "../Framework/Graphics/NinePatch.hpp"
#include "Player.hpp"

const int MAX_TEXT_WIDTH = 200;

class SpeechBubble 
{
public:
    
    SpeechBubble(sf::String &_text, Player *_player);
    void Update();
    void Render(sf::RenderTarget &_target);

    void SetDuration(float _duration);
    bool DurationExceeded()  const;

private:

    float duration;
    Player *player;

    NinePatch bubble;
    sf::Text bubbleText;

    void FitTextIntoMaxWidth();
};