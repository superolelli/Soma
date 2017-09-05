#include "ModelManager.hpp"


void CModelManager::LoadModels(CGameEngine &_engine)
{
	SpriterEngine::Settings::setErrorFunction(SpriterEngine::Settings::simpleError);

	modelOle = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Ole/ole.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetWindow()), new SpriterEngine::ExampleObjectFactory(&_engine.GetWindow()));
}