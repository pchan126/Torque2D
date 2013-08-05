//
//  Layer.cc
//  Torque2D
//
//  Created by Paul L Jan on 2013-06-13.
//

#include "Layer.h"
#include "SceneRenderState.h"

IMPLEMENT_CONOBJECT(Layer);

GFX_ImplementTextureProfile(GFXLayerTextureProfile,
                            GFXTextureProfile::DiffuseMap,
                            GFXTextureProfile::PreserveSize |
                            GFXTextureProfile::Dynamic |
                            GFXTextureProfile::NoMipmap,
                            GFXTextureProfile::None);

Layer::Layer() : mSortMode(SceneRenderQueue::RENDER_SORT_NEWEST),
                 mLight(1.0f, 1.0f, 1.0f, 1.0f),
                 mCameraTranslationScale(1.0f, 1.0f, 1.0f),
                 mRenderFlag(true)
{
   texTarget = GFX->allocRenderToTextureTarget();
};

Layer::~Layer()
{
   
};

void Layer::makeRenderTarget(void)
{
}

void Layer::setRenderTarget()
{
   RectI vPort = GFX->getViewport();
   texHandle = TEXMGR->createTexture(vPort.extent.x, vPort.extent.y, GFXFormatR8G8B8A8, &GFXLayerTextureProfile, 0, 0);
   texTarget->attachTexture(texHandle);
   GFX->setActiveRenderTarget(texTarget);
}

//void Layer::drawRenderTarget()
//{
//   
//}

typeLayerQueryResult Layer::getObjectsinCamera(const SceneRenderState renderState) {
    typeLayerQueryResult result;

//    Vector<SimObject*>::iterator iter = begin();
//    for ( ; iter != end(); iter++ )
//    {
//        SceneObject *obj = dynamic_cast<SceneObject*>(*iter);
//        if (obj)
//        {
//            obj->getIsPointInOOBB(<#(Vector2 const &)worldPoint#>)
//        }
//    }


    return result;
}
