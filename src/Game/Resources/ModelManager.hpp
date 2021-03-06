#pragma once


#include "../../Framework/Patterns/singleton.hpp"

#include "../../Framework/Animations/spriterengine.h"
#include "../../Framework/Animations/sfml_implementation/exampleobjectfactory.h"
#include "../../Framework/Animations/sfml_implementation/examplefilefactory.h"
#include "../../Framework/Animations/global/settings.h"

#include "../../Framework/Gameengine.hpp"

#include "../CombatantID.hpp"

#define g_pModels CModelManager::Get()
class CModelManager : public TSingleton<CModelManager>
{
public:
	//Loads all textures
	void LoadModels(CGameEngine &_engine);
	void Quit();

	SpriterEngine::EntityInstance* GetNewCombatantModelInstance(CombatantID _id);

	SpriterEngine::SpriterModel *modelOleMainRoom;
	SpriterEngine::SpriterModel *modelAnnaMainRoom;
	SpriterEngine::SpriterModel *modelSimonMainRoom;
	SpriterEngine::SpriterModel *modelMarkusMainRoom;

	SpriterEngine::SpriterModel *modelAbilityEffects;
	SpriterEngine::SpriterModel *modelNewBattleAnimation;

private:
	std::vector<SpriterEngine::SpriterModel*> combatantModels;
	std::vector<std::string> instanceNames;
};