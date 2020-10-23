#include "ModelManager.hpp"
#include "ObjectPropertiesManager.hpp"


void CModelManager::LoadModels(CGameEngine &_engine)
{
	SpriterEngine::Settings::setErrorFunction(SpriterEngine::Settings::simpleError);

	modelOleMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Ole2/oleMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelAnnaMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Anna2/annaMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelSimonMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Simon2/simonMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelMarkusMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Markus2/markusMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));

    for(int i = 0; i < CombatantID::number_of_combatants; i++)
        combatantModels.push_back(new SpriterEngine::SpriterModel(g_pObjectProperties->combatantModelFileNames[i], new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget())));

    modelAbilityEffects = new SpriterEngine::SpriterModel("./Data/Sprites/Ability Effects/ability_effects2.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelNewBattleAnimation = new SpriterEngine::SpriterModel("./Data/Sprites/Battle/New Battle Animation/new_battle_animation.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
}



void CModelManager::Quit()
{
	SAFE_DELETE(modelOleMainRoom);
	SAFE_DELETE(modelAnnaMainRoom);
	SAFE_DELETE(modelSimonMainRoom);
	SAFE_DELETE(modelMarkusMainRoom);

    for (auto m : combatantModels)
        SAFE_DELETE(m);

	SAFE_DELETE(modelAbilityEffects);
	SAFE_DELETE(modelNewBattleAnimation);
}