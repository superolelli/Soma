#include "ModelManager.hpp"
#include "../../Framework/Animations/pugixml/pugixml.hpp"


void CModelManager::LoadModels(CGameEngine &_engine)
{
	SpriterEngine::Settings::setErrorFunction(SpriterEngine::Settings::simpleError);
	using namespace pugi;

	xml_document doc;
	doc.load_file("Data/XML/PlayerAttributes.xml");

	for (xml_node& player : doc.child("PlayerAttributes").children())
	{
		instanceNames.push_back(player.name());
		auto modelPath = player.child("Model").text().as_string();
		combatantModels.push_back(new SpriterEngine::SpriterModel(modelPath, new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget())));
	}

	doc.load_file("Data/XML/EnemyAttributes.xml");

	for (xml_node& enemy : doc.child("EnemyAttributes").children())
	{
		instanceNames.push_back(enemy.name());
		auto modelPath = enemy.child("Model").text().as_string();
		combatantModels.push_back(new SpriterEngine::SpriterModel(modelPath, new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget())));
	}

	modelOleMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Ole2/oleMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelAnnaMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Anna2/annaMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelSimonMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Simon2/simonMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelMarkusMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Markus2/markusMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));

    modelAbilityEffects = new SpriterEngine::SpriterModel("./Data/Sprites/Ability Effects/ability_effects.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
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

SpriterEngine::EntityInstance* CModelManager::GetNewCombatantModelInstance(CombatantID _id)
{
	return combatantModels[_id]->getNewEntityInstance(instanceNames[_id]);
}
