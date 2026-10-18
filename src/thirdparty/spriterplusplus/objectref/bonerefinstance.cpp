#include "bonerefinstance.h"

#include "../objectinfo/universalobjectinterface.h"

#include <cmath>

namespace SpriterEngine
{

    BoneRefInstance::BoneRefInstance(UniversalObjectInterface* initialResultObject,
                                     TransformProcessor* initialParentTransformer,
                                     TransformProcessor* initialSelfTransformer,
                                     TimelineKey* initialKey)
        : ObjectRefInstance(initialResultObject, initialParentTransformer, initialKey)
        , transformProcessor(initialSelfTransformer)
    {
    }

    void BoneRefInstance::processTransform()
    {
        ObjectRefInstance::processTransform();
        transformProcessor->setTrigFunctions();
    }
} // namespace SpriterEngine
