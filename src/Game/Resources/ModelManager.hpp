#pragma once


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../../Framework/Patterns/singleton.hpp"

#include "../../Framework/Animations/spriterengine.h"
#include "../../Framework/Animations/sfml_implementation/exampleobjectfactory.h"
#include "../../Framework/Animations/sfml_implementation/examplefilefactory.h"
#include "../../Framework/Animations/global/settings.h"

#include "../../Framework/Gameengine.hpp"

#define g_pModels CModelManager::Get()
class CModelManager : public TSingleton<CModelManager>
{
public:
	//Loads all textures
	void LoadModels(CGameEngine &_engine);
	void Quit();

	SpriterEngine::SpriterModel *modelOle;
	SpriterEngine::SpriterModel *modelAnna;
	SpriterEngine::SpriterModel *modelSimon;
	SpriterEngine::SpriterModel *modelMarkus;

	SpriterEngine::SpriterModel *modelAbtruenniger;
	SpriterEngine::SpriterModel *modelGesetzloser;
};