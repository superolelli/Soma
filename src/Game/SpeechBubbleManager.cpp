#include "SpeechBubbleManager.hpp"
#include "Resources/StringManager.hpp"


SpeechBubbleManager::SpeechBubbleManager(CGameEngine* _engine, AdventureGroup* _adventureGroup)
    : engine(_engine)
    , adventureGroup(_adventureGroup)
    , speechBubblesActive(true)
    , speechBubbleTimer(1.0f)
{
}


void SpeechBubbleManager::Update()
{
    if (speechBubblesActive)
    {
        speechBubbleTimer -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();
        if (speechBubbleTimer <= 0.0f)
        {
            speechBubbleTimer = 1.0f;
            if (rand() % 10 == 0)
                CreateSpeechBubble();
        }
    }

    for (auto i = currentSpeechBubbles.begin(); i != currentSpeechBubbles.end();)
    {
        (*i)->Update();
        if ((*i)->DurationExceeded())
        {
            SAFE_DELETE(*i);
            i = currentSpeechBubbles.erase(i);
            continue;
        }

        i++;
    }
}


void SpeechBubbleManager::Render()
{
    if (speechBubblesActive) 
    {
        for (auto s : currentSpeechBubbles)
            s->Render(engine->GetRenderTarget());
    }
}

void SpeechBubbleManager::ActivateSpeechBubbles()
{
    speechBubblesActive = true;
}

void SpeechBubbleManager::DeactivateSpeechBubbles()
{
    speechBubblesActive = false;
}


void SpeechBubbleManager::CreateSpeechBubble()
{
    int playerId = rand() % 4;
    int speechId = rand()% g_pStringContainer->speechBubbleContents[playerId].size();
    SpeechBubble *bubble = new SpeechBubble(g_pStringContainer->speechBubbleContents[playerId][speechId], adventureGroup->GetPlayer(playerId));
    bubble->SetDuration(3.0f);
    currentSpeechBubbles.push_back(bubble);
}