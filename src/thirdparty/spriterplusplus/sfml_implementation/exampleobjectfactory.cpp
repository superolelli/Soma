#include "exampleobjectfactory.h"

#include "sfmlboneinstanceinfo.h"
#include "sfmlboxinstanceinfo.h"
#include "sfmlpointinstanceinfo.h"
#include "sfmlsoundobjectinforeference.h"

namespace SpriterEngine
{

    ExampleObjectFactory::ExampleObjectFactory(sf::RenderTarget* validRenderWindow)
        : renderWindow(validRenderWindow)
    {
    }

    PointInstanceInfo* ExampleObjectFactory::newPointInstanceInfo()
    {
        return new SfmlPointInstanceInfo(renderWindow);
    }

    BoxInstanceInfo* ExampleObjectFactory::newBoxInstanceInfo(point size)
    {
        return new SfmlBoxInstanceInfo(size, renderWindow);
    }

    BoneInstanceInfo* ExampleObjectFactory::newBoneInstanceInfo(point size)
    {
        return new SfmlBoneInstanceInfo(size, renderWindow);
    }

} // namespace SpriterEngine