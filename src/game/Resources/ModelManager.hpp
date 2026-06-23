#pragma once


#include "../../engine/Patterns/singleton.hpp"

#include "../../../third_party/SpriterPlusPlus/spriterengine.h"
#include "../../../third_party/SpriterPlusPlus/sfml_implementation/exampleobjectfactory.h"
#include "../../../third_party/SpriterPlusPlus/sfml_implementation/examplefilefactory.h"
#include "../../../third_party/SpriterPlusPlus/global/settings.h"

#include "../../engine/Gameengine.hpp"

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