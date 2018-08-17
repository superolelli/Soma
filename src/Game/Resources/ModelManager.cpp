#include "ModelManager.hpp"


void CModelManager::LoadModels(CGameEngine &_engine)
{
	SpriterEngine::Settings::setErrorFunction(SpriterEngine::Settings::simpleError);

	modelOle = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Ole2/ole2.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
	modelAnna = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Anna2/anna.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
	modelSimon = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Simon2/simon.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
	modelMarkus = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Markus2/markus.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
	
	modelAbtruenniger = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Abtruenniger/abtruenniger.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
	modelGesetzloser = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Gesetzloser/gesetzloser.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
	modelAbilityEffects = new SpriterEngine::SpriterModel("./Data/Sprites/Ability Effects/ability_effects.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
}



void CModelManager::Quit()
{
	SAFE_DELETE(modelOle);
	SAFE_DELETE(modelAnna);
	SAFE_DELETE(modelSimon);
	SAFE_DELETE(modelMarkus);

	SAFE_DELETE(modelAbtruenniger);
	SAFE_DELETE(modelGesetzloser);
	SAFE_DELETE(modelAbilityEffects);
}