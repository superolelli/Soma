#pragma once


#include "../Framework/Graphics/NinePatch.hpp"
#include "AdventureGroup.hpp"
#include "SpeechBubble.h"


class SpeechBubbleManager {
public:

    SpeechBubbleManager(CGameEngine* _engine, AdventureGroup* _adventureGroup);
    void Update();
    void Render();

    void ActivateSpeechBubbles();
    void DeactivateSpeechBubbles();

private:

    bool speechBubblesActive;
    float speechBubbleTimer;

    void CreateSpeechBubble();

    CGameEngine* engine;
    AdventureGroup* adventureGroup;

    std::vector<SpeechBubble*> currentSpeechBubbles;
};