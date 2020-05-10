#include "ModelManager.hpp"


void CModelManager::LoadModels(CGameEngine &_engine)
{
	SpriterEngine::Settings::setErrorFunction(SpriterEngine::Settings::simpleError);

	modelOleMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Ole2/oleMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelAnnaMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Anna2/annaMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelSimonMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Simon2/simonMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelMarkusMainRoom = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Markus2/markusMainRoom.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));

	modelOle = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Ole2/ole2.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelAnna = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Anna2/anna.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelSimon = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Simon2/simon.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelMarkus = new SpriterEngine::SpriterModel("./Data/Sprites/Players/Markus2/markus.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	
	modelAbtruenniger = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Abtruenniger/abtruenniger.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelGesetzloser = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Gesetzloser/gesetzloser.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelIndianer = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Indianer/indianer.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelHilfssheriff = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Hilfssheriff/hilfssheriff.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelGreg = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Greg Digger/greg.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelApacheKid = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Apache Kid/ApacheKid.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelBigSpencer= new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Big Spencer/BigSpencer.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelTequilaJoe = new SpriterEngine::SpriterModel("./Data/Sprites/Enemies/Bang/Tequila Joe/TequilaJoe.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelAbilityEffects = new SpriterEngine::SpriterModel("./Data/Sprites/Ability Effects/ability_effects.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
	modelNewBattleAnimation = new SpriterEngine::SpriterModel("./Data/Sprites/Battle/New Battle Animation/new_battle_animation.scml", new SpriterEngine::ExampleFileFactory(&_engine.GetRenderTarget()), new SpriterEngine::ExampleObjectFactory(&_engine.GetRenderTarget()));
}



void CModelManager::Quit()
{
	SAFE_DELETE(modelOleMainRoom);
	SAFE_DELETE(modelAnnaMainRoom);
	SAFE_DELETE(modelSimonMainRoom);
	SAFE_DELETE(modelMarkusMainRoom);

	SAFE_DELETE(modelOle);
	SAFE_DELETE(modelAnna);
	SAFE_DELETE(modelSimon);
	SAFE_DELETE(modelMarkus);

	SAFE_DELETE(modelAbtruenniger);
	SAFE_DELETE(modelGesetzloser);
	SAFE_DELETE(modelIndianer);
	SAFE_DELETE(modelHilfssheriff);
	SAFE_DELETE(modelGreg);
	SAFE_DELETE(modelApacheKid);
	SAFE_DELETE(modelBigSpencer);
	SAFE_DELETE(modelTequilaJoe);
	SAFE_DELETE(modelAbilityEffects);
	SAFE_DELETE(modelNewBattleAnimation);
}