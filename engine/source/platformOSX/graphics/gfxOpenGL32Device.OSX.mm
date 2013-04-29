//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#import "platform/platform.h"
#import "platformOSX/platformOSX.h"
#include "./gfxOpenGL32Device.h"

//#include "graphics/gfxCubemap.h"
#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxInit.h"

#include "./gfxOpenGL32EnumTranslate.h"
#include "./gfxOpenGL32VertexBuffer.h"
#include "./gfxOpenGL32TextureTarget.h"
#include "./gfxOpenGL32TextureManager.h"
#include "./gfxOpenGL32TextureObject.h"
//#include "./gfxOpenGLCubemap.h"
#include "./gfxOpenGL32CardProfiler.h"
#include "./gfxOpenGL32WindowTarget.h"

#include "./gfxOpenGL32Shader.h"
#include "graphics/primBuilder.h"
#include "console/console.h"

//#include "./gfxOpenGLOcclusionQuery.h"

GFXAdapter::CreateDeviceInstanceDelegate GFXOpenGL32Device::mCreateDeviceInstance(GFXOpenGL32Device::createInstance);

GFXDevice *GFXOpenGL32Device::createInstance( U32 adapterIndex )
{
    return new GFXOpenGL32Device(adapterIndex);
}

#include "osxGLUtils.h"

void GFXOpenGL32Device::initGLState()
{
    // Currently targeting OpenGL 3.2 (Mac)
    
    // We don't currently need to sync device state with a known good place because we are
    // going to set everything in GFXOpenGL32StateBlock, but if we change our GFXOpenGL32StateBlock strategy, this may
    // need to happen.
    
    // Deal with the card profiler here when we know we have a valid context.
    mCardProfiler = new GFXOpenGLCardProfiler();
    mCardProfiler->init();

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&mMaxShaderTextures);
//    glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint*)&mMaxFFTextures);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    mPixelShaderVersion = 2.0f;
    
    // MACHAX - Setting mPixelShaderVersion to 3.0 will allow Advanced Lighting
    // to run.  At the time of writing (6/18) it doesn't quite work yet.
    if(Con::getBoolVariable("$pref::machax::enableAdvancedLighting", false))
        mPixelShaderVersion = 3.0f;
    
//    mSupportsAnisotropic = mCardProfiler->queryProfile( "GL::EXT_texture_filter_anisotropic" );
}


//-----------------------------------------------------------------------------
// Matrix interface
GFXOpenGL32Device::GFXOpenGL32Device( U32 adapterIndex )  : GFXOpenGLDevice( adapterIndex ),
                        mAdapterIndex(adapterIndex),
                        mCurrentVB(NULL),
                        m_mCurrentWorld(true),
                        m_mCurrentView(true),
                        mContext(nil),
                        mPixelFormat(NULL),
                        mPixelShaderVersion(0.0f),
                        mMaxShaderTextures(2),
                        mMaxFFTextures(2),
                        mClip(0, 0, 0, 0),
                        mTextureLoader(NULL)
{
    GFXOpenGLEnumTranslate::init();
   
    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
    
    m_WorldStack.push_back(MatrixF(true));
    m_ProjectionStack.push_back(MatrixF(true));
    
    mDeviceName = "OpenGL32";
    mFullScreenOnly = false;
}


GFXOpenGL32Device::~GFXOpenGL32Device()
{
    [(NSOpenGLContext*)mContext release];
}

static String _getRendererForDisplay(CGDirectDisplayID display)
{
    Vector<NSOpenGLPixelFormatAttribute> attributes = _createStandardPixelFormatAttributesForDisplay(display);
    
    NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes.address()];
    AssertFatal(fmt, "_getRendererForDisplay - Unable to create a pixel format object");
    attributes.clear();
    
    NSOpenGLContext* ctx = [[[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil] retain];
    [fmt release];
    AssertFatal(ctx, "_getRendererForDisplay - Unable to create an OpenGL context");
    
    // Save the current context, just in case
    NSOpenGLContext* currCtx = [NSOpenGLContext currentContext];
    [ctx makeCurrentContext];
    
    // get the renderer string
    String ret((const char*)glGetString(GL_RENDERER));
    
    // Restore our old context, release the context and pixel format.
    [currCtx makeCurrentContext];
    [ctx release];
    return ret;
}


void GFXOpenGL32Device::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    if(!mInitialized)
    {
       AssertFatal(!mContext && !mPixelFormat, "_createInitialContextAndFormat - Already created initial context and format");
       
       mPixelFormat = generateValidPixelFormat(mode.fullScreen, mode.bitDepth, 0);
       AssertFatal(mPixelFormat, "_createInitialContextAndFormat - Unable to create an OpenGL pixel format");
       
       mContext = [[[NSOpenGLContext alloc] initWithFormat: (NSOpenGLPixelFormat*)mPixelFormat shareContext: nil] retain];
       AssertFatal(mContext, "_createInitialContextAndFormat - Unable to create an OpenGL context");

       [mContext makeCurrentContext];
       
       mTextureManager = new GFXOpenGL32TextureManager(mContext);

       initGLState();
       initGenericShaders();
       mInitialized = true;
       deviceInited();
    }
}

void GFXOpenGL32Device::addVideoMode(GFXVideoMode toAdd)
{
    // Only add this resolution if it is not already in the list:
    mVideoModes.push_back_unique( toAdd );
}

void addVideoModeCallback( const void *value, void *context )
{
	if (CFGetTypeID (value) == CGDisplayModeGetTypeID())
    {
        CGDisplayModeRef mode = (CGDisplayModeRef)value;
        Vector<GFXVideoMode> *videoModes = (Vector<GFXVideoMode> *)context;
        GFXVideoMode toAdd;

        toAdd.resolution.x = CGDisplayModeGetWidth(mode); //  valueForKey:@"Width"] intValue];
        toAdd.resolution.y = CGDisplayModeGetHeight(mode); //  [[mode valueForKey:@"Height"] intValue];
        CFStringRef pixCode = CGDisplayModeCopyPixelEncoding(mode);
        toAdd.bitDepth = CFStringGetLength(pixCode); // [[mode valueForKey:@"BitsPerPixel"] intValue];
        toAdd.refreshRate = CGDisplayModeGetRefreshRate(mode); // [[mode valueForKey:@"RefreshRate"] intValue];
        
        toAdd.fullScreen = false;
        
        CFRelease(pixCode);
        // skip if mode claims to be 8bpp
        if( toAdd.bitDepth == 8 )
            return;
        
        videoModes->push_back_unique(toAdd);
    }
}

void GFXOpenGL32Device::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
    GFXAdapter *toAdd;
    
    Vector<GFXVideoMode> videoModes;
    
    CGDirectDisplayID display = CGMainDisplayID();
    
    // Enumerate all available resolutions: // depreciated use CGDisplayCopyAllDisplayModes
    CFArrayRef modeArray = CGDisplayCopyAllDisplayModes( display, NULL );
    CFArrayApplyFunction(modeArray, CFRangeMake(0,CFArrayGetCount(modeArray)), addVideoModeCallback, &videoModes);
    
    // Get number of displays
    CGDisplayCount dispCnt;
    CGGetActiveDisplayList(0, NULL, &dispCnt);
    
    // Take advantage of GNU-C
    CGDirectDisplayID displays[dispCnt];
    
    CGGetActiveDisplayList(dispCnt, displays, &dispCnt);
    for(U32 i = 0; i < dispCnt; i++)
    {
        toAdd = new GFXAdapter();
        toAdd->mType = OpenGL;
        toAdd->mIndex = (U32)displays[i];
        toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;
        String renderer = _getRendererForDisplay(displays[i]);
        AssertFatal(dStrlen(renderer.c_str()) < GFXAdapter::MaxAdapterNameLen, "GFXGLDevice::enumerateAdapter - renderer name too long, increae the size of GFXAdapter::MaxAdapterNameLen (or use String!)");
        dStrncpy(toAdd->mName, renderer.c_str(), GFXAdapter::MaxAdapterNameLen);
        adapterList.push_back(toAdd);
        
        for (S32 j = videoModes.size() - 1 ; j >= 0 ; j--)
            toAdd->mAvailableModes.push_back(videoModes[j]);
    }
}

void GFXOpenGL32Device::enumerateVideoModes()
{
    mVideoModes.clear();
    CGDirectDisplayID display = CGMainDisplayID();
    
    // Enumerate all available resolutions:
    CFArrayRef modeArray = CGDisplayCopyAllDisplayModes( display, NULL );
    CFArrayApplyFunction(modeArray, CFRangeMake(0,CFArrayGetCount(modeArray)), addVideoModeCallback, &mVideoModes);
}



inline void GFXOpenGL32Device::pushWorldMatrix()
{
    mWorldMatrixDirty = true;
    mStateDirty = true;
    
    MatrixF newMatrix = m_WorldStack.last();
    m_WorldStack.push_back(newMatrix);
//    GLKMatrixStackPush(m_WorldStackRef);
}

inline void GFXOpenGL32Device::popWorldMatrix()
{
    mWorldMatrixDirty = true;
    mStateDirty = true;
    
    m_WorldStack.pop_back();
//    GLKMatrixStackPop(m_WorldStackRef);
}

inline void GFXOpenGL32Device::multWorld( const MatrixF &mat )
{
    mWorldMatrixDirty = true;
    mStateDirty = true;
    
    MatrixF newMatrix = m_WorldStack.last();
    newMatrix*=mat;
    m_WorldStack.last() = newMatrix;
//    GLKMatrixStackMultiplyMatrix4(m_WorldStackRef, GLKMatrix4MakeWithArray(mat));
}

//inline void GFXOpenGL32Device::setTextureMatrix( const U32 stage, const MatrixF &texMat )
//{
//    AssertFatal( stage < TEXTURE_STAGE_COUNT, "Out of range texture sampler" );
//    mStateDirty = true;
//    mTextureMatrixDirty[stage] = true;
//    mTextureMatrix[stage] = texMat;
//    mTextureMatrixCheckDirty = true;
//}



void GFXOpenGL32Device::zombify()
{
    mTextureManager->zombify();
    if(mCurrentVB)
        mCurrentVB->finish();
    //mVolatileVBs.clear();
    //mVolatilePBs.clear();
    GFXResource* walk = mResourceListHead;
    while(walk)
    {
        walk->zombify();
        walk = walk->getNextResource();
    }
}

void GFXOpenGL32Device::resurrect()
{
    GFXResource* walk = mResourceListHead;
    while(walk)
    {
        walk->resurrect();
        walk = walk->getNextResource();
    }
    if(mCurrentVB)
        mCurrentVB->prepare();
    mTextureManager->resurrect();
}


GFXVertexBuffer* GFXOpenGL32Device::findVolatileVBO(U32 numVerts, const GFXVertexFormat *vertexFormat, U32 vertSize, void* data)
{
    for(U32 i = 0; i < mVolatileVBs.size(); i++)
        if (  mVolatileVBs[i]->mVertexCount >= numVerts &&
            mVolatileVBs[i]->mVertexFormat.isEqual( *vertexFormat ) &&
            mVolatileVBs[i]->mVertexSize == vertSize &&
            mVolatileVBs[i]->getRefCount() == 1 )
        {
            mVolatileVBs[i].getPointer()->set(data, numVerts*vertSize);
            return mVolatileVBs[i];
        }
    
    // No existing VB, so create one
    StrongRefPtr<GFXOpenGL32VertexBuffer> buf(new GFXOpenGL32VertexBuffer(GFX, numVerts, vertexFormat, vertSize, GFXBufferTypeVolatile, data));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}


GFXVertexBuffer *GFXOpenGL32Device::allocVertexBuffer(   U32 vertexCount,
                                                  const GFXVertexFormat *vertexFormat,
                                                  U32 vertSize,
                                                  GFXBufferType bufferType,
                                                  void *vertexBuffer,
                                                    U32 indexCount,
                                                    void *indexBuffer)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(vertexCount, vertexFormat, vertSize, vertexBuffer);
   
    GFXOpenGL32VertexBuffer* buf = new GFXOpenGL32VertexBuffer( GFX, vertexCount, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
    buf->registerResourceWithDevice(this);
    return buf;
}


void GFXOpenGL32Device::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
    if (stream > 0) return;
    
    AssertFatal( stream == 0, "GFXOpenGL32Device::setVertexStream - We don't support multiple vertex streams!" );
    
    // Reset the state the old VB required, then set the state the new VB requires.
    if ( mCurrentVB )
        mCurrentVB->finish();
    
    mCurrentVB = static_cast<GFXOpenGL32VertexBuffer*>( buffer );
    if ( mCurrentVB )
        mCurrentVB->prepare();
}

void GFXOpenGL32Device::setVertexStreamFrequency( U32 stream, U32 frequency )
{
    // We don't support vertex stream frequency or mesh instancing in OGL yet.
}

//GFXCubemap* GFXOpenGL32Device::createCubemap()
//{
////    GFXOpenGLCubemap* cube = new GFXOpenGLCubemap();
////    cube->registerResourceWithDevice(this);
////    return cube;
//};

void GFXOpenGL32Device::clear(U32 flags, ColorI color, F32 z, U32 stencil)
{
    // Make sure we have flushed our render target state.
    _updateRenderTargets();
    
    bool zwrite = true;
    //   if (mCurrentGLStateBlock)
    //   {
    //      zwrite = mCurrentGLStateBlock->getDesc().zWriteEnable;
    //   }
    
    glDepthMask(true);
    
    GLbitfield clearflags = 0;
    clearflags |= (flags & GFXClearTarget)   ? GL_COLOR_BUFFER_BIT : 0;
    clearflags |= (flags & GFXClearZBuffer)  ? GL_DEPTH_BUFFER_BIT : 0;
    clearflags |= (flags & GFXClearStencil)  ? GL_STENCIL_BUFFER_BIT : 0;
    
    glClear(clearflags);
    
    ColorF c = color;
    glClearDepth(z);
    glClearStencil(stencil);
    glClearColor(c.red, c.green, c.blue, c.alpha);

    if(!zwrite)
        glDepthMask(false);
}



void GFXOpenGL32Device::updateStates(bool forceSetAll /*=false*/)
{
    PROFILE_SCOPE(GFXDevice_updateStates);
    
    if(forceSetAll)
    {
        bool rememberToEndScene = false;
        if(!canCurrentlyRender())
        {
            if (!beginScene())
            {
                AssertFatal(false, "GFXDevice::updateStates:  Unable to beginScene!");
            }
            rememberToEndScene = true;
        }
        
        setVertexDecl( mCurrVertexDecl );
        
        for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
        {
            setVertexStream( i, mCurrentVertexBuffer[i] );
        }
        
        /// Stateblocks
        if ( mNewStateBlock )
            setStateBlockInternal(mNewStateBlock, true);
        mCurrentStateBlock = mNewStateBlock;
        
        for(U32 i = 0; i < getNumSamplers(); i++)
        {
            switch (mTexType[i])
            {
                case GFXTDT_Normal :
                {
                    mCurrentTexture[i] = mNewTexture[i];
                    setTextureInternal(i, mCurrentTexture[i]);
                }
                    break;
//                case GFXTDT_Cube :
//                {
//                    mCurrentCubemap[i] = mNewCubemap[i];
//                    if (mCurrentCubemap[i])
//                        mCurrentCubemap[i]->setToTexUnit(i);
//                    else
//                        setTextureInternal(i, NULL);
//                }
//                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }
        
//        // Set our material
//        setLightMaterialInternal(mCurrentLightMaterial);
        
//        // Set our lights
//        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
//        {
//            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
//        }
        
        _updateRenderTargets();
        
        if(rememberToEndScene)
            endScene();
        
        return;
    }
    
    if (!mStateDirty)
        return;
    
    // Normal update logic begins here.
    mStateDirty = false;
    
   // Update Projection Matrix
   if( mProjectionMatrixDirty )
   {
//       MatrixF temp(GLKMatrixStackGetMatrix4(m_ProjectionStackRef).m);
       MatrixF temp = m_ProjectionStack.last();
//       temp.transpose();
        setMatrix( GFXMatrixProjection, temp);
        mProjectionMatrixDirty = false;
   }
    
   // Update World Matrix
   if( mWorldMatrixDirty)
   {
//       MatrixF temp(GLKMatrixStackGetMatrix4(m_WorldStackRef).m);
//       temp.transpose();
//       if ( mWorldMatrixDirty) // && !mViewMatrixDirty)
//           m_mCurrentView = temp.inverse() * m_mCurrentWorld;
//        else
       MatrixF temp = m_WorldStack.last();
       m_mCurrentWorld = temp;

       mWorldMatrixDirty = false;
   }
    
    if ( mViewMatrixDirty )
    {
        mViewMatrixDirty = false;
    }
    
    // Update the vertex declaration.
    if ( mVertexDeclDirty )
    {
        setVertexDecl( mCurrVertexDecl );
        mVertexDeclDirty = false;
    }
    
    // Update the vertex buffers.
    for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
    {
        if ( mVertexBufferDirty[i] )
        {
            setVertexStream( i, mCurrentVertexBuffer[i] );
            mVertexBufferDirty[i] = false;
        }
        
        if ( mVertexBufferFrequencyDirty[i] )
        {
            setVertexStreamFrequency( i, mVertexBufferFrequency[i] );
            mVertexBufferFrequencyDirty[i] = false;
        }
    }
    
    // NOTE: With state blocks, it's now important to update state before setting textures
    // some devices (e.g. OpenGL) set states on the texture and we need that information before
    // the texture is activated.
    if (mStateBlockDirty)
    {
        setStateBlockInternal(mNewStateBlock, false);
        mCurrentStateBlock = mNewStateBlock;
        mStateBlockDirty = false;
    }
    
    if( mTexturesDirty )
    {
        mTexturesDirty = false;
        for(U32 i = 0; i < getNumSamplers(); i++)
        {
            if(!mTextureDirty[i])
                continue;
            mTextureDirty[i] = false;
            
            switch (mTexType[i])
            {
                case GFXTDT_Normal :
                {
                    mCurrentTexture[i] = mNewTexture[i];
                    setTextureInternal(i, mCurrentTexture[i]);
                }
                    break;
//                case GFXTDT_Cube :
//                {
//                    mCurrentCubemap[i] = mNewCubemap[i];
//                    if (mCurrentCubemap[i])
//                        mCurrentCubemap[i]->setToTexUnit(i);
//                    else
//                        setTextureInternal(i, NULL);
//                }
//                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }
    }
    
//    // Set light material
//    if(mLightMaterialDirty)
//    {
//        setLightMaterialInternal(mCurrentLightMaterial);
//        mLightMaterialDirty = false;
//    }
//    
//    // Set our lights
//    if(mLightsDirty)
//    {
//        mLightsDirty = false;
//        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
//        {
//            if(!mLightDirty[i])
//                continue;
//            
//            mLightDirty[i] = false;
//            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
//        }
//    }
    
    _updateRenderTargets();
    
#ifdef TORQUE_DEBUG_RENDER
    doParanoidStateCheck();
#endif
}


void GFXOpenGL32Device::setTextureInternal(U32 textureUnit, const GFXTextureObject *texture)
{
    const GFXOpenGL32TextureObject *tex = static_cast<const GFXOpenGL32TextureObject*>(texture);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    if (tex)
    {
        // GFXOpenGL32TextureObject::bind also handles applying the current sampler state.
        if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
        {
            glBindTexture(mActiveTextureType[textureUnit], 0);
        }
        mActiveTextureType[textureUnit] = tex->getBinding();
        tex->bind(textureUnit);
    }
    else if(mActiveTextureType[textureUnit] != GL_ZERO)
    {
        glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        mActiveTextureType[textureUnit] = GL_ZERO;
    }
    glActiveTexture(GL_TEXTURE0);
}

//void GFXOpenGL32Device::setCubemapInternal(U32 textureUnit, const GFXOpenGLCubemap* texture)
//{
//    glActiveTexture(GL_TEXTURE0 + textureUnit);
//    if(texture)
//    {
//        if(mActiveTextureType[textureUnit] != GL_TEXTURE_CUBE_MAP && mActiveTextureType[textureUnit] != GL_ZERO)
//            glBindTexture(mActiveTextureType[textureUnit], 0);
//
//        mActiveTextureType[textureUnit] = GL_TEXTURE_CUBE_MAP;
//        texture->bind(textureUnit);
//    }
//    else if(mActiveTextureType[textureUnit] != GL_ZERO)
//    {
//        glBindTexture(mActiveTextureType[textureUnit], 0);
//        mActiveTextureType[textureUnit] = GL_ZERO;
//    }
//    
//    glActiveTexture(GL_TEXTURE0);
//}

void GFXOpenGL32Device::setMatrix( GFXMatrixType mtype, const MatrixF &mat )
{
    MatrixF modelview;
    switch (mtype)
    {
        case GFXMatrixWorld :
        {
            m_mCurrentWorld = mat;
            modelview = m_mCurrentWorld;
            m_WorldStack.last() = mat;
//            modelview *= m_mCurrentView;
//            GLKMatrixStackLoadMatrix4(m_WorldStackRef, GLKMatrix4MakeWithArrayAndTranspose(modelview));
        }
            break;
        case GFXMatrixView :
        {
            m_mCurrentView = mat;
//            m_ProjectionStack.last() = mat;
//            modelview = m_mCurrentView;
//            modelview *= m_mCurrentWorld;
//            GLKMatrixStackLoadMatrix4(m_WorldStackRef, GLKMatrix4MakeWithArrayAndTranspose(modelview));
//            m_m
        }
            break;
        case GFXMatrixProjection :
        {
            m_mCurrentProj = mat;
            m_ProjectionStack.last() = mat;
//            GLKMatrixStackLoadMatrix4(m_ProjectionStackRef, GLKMatrix4MakeWithArrayAndTranspose(mat));
        }
            break;
            // CodeReview - Add support for texture transform matrix types
        default:
            AssertFatal(false, "GFXOpenGL32Device::setMatrix - Unknown matrix mode!");
            return;
    }
}


const MatrixF GFXOpenGL32Device::getMatrix( GFXMatrixType mtype )
{
    MatrixF ret = MatrixF(true);
    switch (mtype)
    {
        case GFXMatrixWorld :
        {
            return m_mCurrentWorld;
        }
            break;
        case GFXMatrixView :
        {
            return m_mCurrentView;
        }
            break;
        case GFXMatrixProjection :
        {
            return m_mCurrentProj;
        }
            break;
            // CodeReview - Add support for texture transform matrix types
        default:
            AssertFatal(false, "GFXOpenGL32Device::setMatrix - Unknown matrix mode!");
    }
    return ret;
}

void GFXOpenGL32Device::setClipRect( const RectI &inRect )
{
    AssertFatal(mCurrentRT.isValid(), "GFXOpenGL32Device::setClipRect - must have a render target set to do any rendering operations!");
    
    // Clip the rect against the renderable size.
    Point2I size = mCurrentRT->getSize();
    RectI maxRect(Point2I(0,0), size);
    mClip = inRect;
    mClip.intersect(maxRect);
    
    // Create projection matrix.  See http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/ortho.html
    const F32 left = mClip.point.x;
    const F32 right = mClip.point.x + mClip.extent.x;
    const F32 bottom = mClip.extent.y;
    const F32 top = 0.0f;
    const F32 near = 0.0f;
    const F32 far = 1.0f;
    
    m_mCurrentProj.setOrtho(left, right, bottom, top, near, far);
    m_mCurrentProj.translate(0.0, -mClip.point.y, 0.0f);
    setMatrix(GFXMatrixProjection, m_mCurrentProj);
    
    MatrixF mTempMatrix(true);
    setViewMatrix( mTempMatrix );
    setWorldMatrix( mTempMatrix );
    
    // Set the viewport to the clip rect
    RectI viewport(mClip.point.x, size.y - (mClip.point.y + mClip.extent.y), mClip.extent.x, mClip.extent.y);
    setViewport(viewport);
}


/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXOpenGL32Device::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
    return GFXStateBlockRef(new GFXOpenGL32StateBlock(desc));
}

/// Activates a stateblock
void GFXOpenGL32Device::setStateBlockInternal(GFXStateBlock* block, bool force)
{
    AssertFatal(dynamic_cast<GFXOpenGL32StateBlock*>(block), "GFXOpenGL32Device::setStateBlockInternal - Incorrect stateblock type for this device!");
    GFXOpenGL32StateBlock* glBlock = static_cast<GFXOpenGL32StateBlock*>(block);
    GFXOpenGL32StateBlock* glCurrent = static_cast<GFXOpenGL32StateBlock*>(mCurrentStateBlock.getPointer());
    if (force)
        glCurrent = NULL;
    
    glBlock->activate(glCurrent); // Doesn't use current yet.
    mCurrentGLStateBlock = glBlock;
}

////------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GFXWindowTarget *GFXOpenGL32Device::allocWindowTarget(PlatformWindow *window)
{
   if (window == NULL)
      return NULL;
   
   NSOpenGLView* view = (NSOpenGLView*)window->getPlatformDrawable();
   AssertFatal([view isKindOfClass:[NSOpenGLView class]], avar("_createContextForWindow - Supplied a %s instead of a NSOpenGLView", [[view className] UTF8String]));
   
   NSOpenGLContext* ctx = nil;
   ctx = [[[ NSOpenGLContext alloc] initWithFormat:mPixelFormat shareContext:mContext] autorelease];
   
   AssertFatal(ctx, "Unable to create a shared OpenGL context");
   if (ctx != nil)
   {
      [view setPixelFormat: (NSOpenGLPixelFormat*)mPixelFormat];
      [view setOpenGLContext: ctx];
   }
   
    // Allocate the wintarget and create a new context.
    GFXOpenGL32WindowTarget *gwt = new GFXOpenGL32WindowTarget(window, this);
    gwt->mContext = ctx ? ctx : mContext;
    return gwt;
}


GFXTextureTarget * GFXOpenGL32Device::allocRenderToTextureTarget()
{
    GFXOpenGL32TextureTarget *targ = new GFXOpenGL32TextureTarget();
    targ->registerResourceWithDevice(this);
    return targ;
}

//GFXOcclusionQuery* GFXOpenGL32Device::createOcclusionQuery()
//{
//    GFXOcclusionQuery *query = new GFXOpenGLOcclusionQuery( this );
//    query->registerResourceWithDevice(this);
//    return query;
//}

void GFXOpenGL32Device::initGenericShaders()
{
    Vector<GFXShaderMacro> macros;
    char vertBuffer[1024];
    char fragBuffer[1024];
    //  #Color Shader
    
    const char* shaderDirectory = Con::getVariable("$GUI::shaderDirectory");
    Con::printf("loading shaders from %s", shaderDirectory);
    
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/C.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/C.fsh", shaderDirectory);

    mGenericShader[0] = createShader();
    mGenericShader[0]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[0] = mGenericShader[0]->allocConstBuffer();
    
    //  #Texture Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/simple.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/simple.fsh", shaderDirectory);
    
    mGenericShader[1] = createShader();
    mGenericShader[1]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[1] = mGenericShader[1]->allocConstBuffer();
    
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/point.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/point.fsh", shaderDirectory);
    
    mGenericShader[2] = createShader();
    mGenericShader[2]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[2] = mGenericShader[2]->allocConstBuffer();
    
    //    GFXShaderConstHandle* hand = mGenericShader[0]->getShaderConstHandle("$mvp_matrix");
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/test.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/test.fsh", shaderDirectory);
    
    mGenericShader[3] = createShader();
    mGenericShader[3]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[3] = mGenericShader[3]->allocConstBuffer();

    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/alpha.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/alpha.fsh", shaderDirectory);
    
    mGenericShader[4] = createShader();
    mGenericShader[4]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[4] = mGenericShader[4]->allocConstBuffer();
}


void GFXOpenGL32Device::setupGenericShaders( GenericShaderType type )
{
    MatrixF xform(GFX->getProjectionMatrix());
    xform *= GFX->getViewMatrix();
    xform *= GFX->getWorldMatrix();
    xform.transpose();
    
//    Con::printf("setupGenericShaders");
//    Con::printf("%f %f %f %f", xform[0], xform[1], xform[2], xform[3]);
//    Con::printf("%f %f %f %f", xform[4], xform[5], xform[6], xform[7]);
//    Con::printf("%f %f %f %f", xform[8], xform[9], xform[10], xform[11]);
//    Con::printf("%f %f %f %f", xform[12], xform[13], xform[14], xform[15]);
    
    
    switch (type) {
        case GSColor:
            setShader(mGenericShader[0]);
            setShaderConstBuffer( mGenericShaderConst[0] );
            mGenericShaderConst[0]->setSafe( mGenericShader[0]->getShaderConstHandle("$mvp_matrix"), xform );
            break;
        case GSTexture:
        case GSModColorTexture:
        case GSAddColorTexture:
            setShader(mGenericShader[1]);
            setShaderConstBuffer( mGenericShaderConst[1] );
            mGenericShaderConst[1]->setSafe( mGenericShader[1]->getShaderConstHandle("$mvp_matrix"), xform );
            mGenericShaderConst[1]->setSafe( mGenericShader[1]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
        case GSPoint:
            setShader(mGenericShader[2]);
            setShaderConstBuffer( mGenericShaderConst[2] );
            mGenericShaderConst[2]->setSafe( mGenericShader[2]->getShaderConstHandle("$mvp_matrix"), xform );
            mGenericShaderConst[2]->setSafe( mGenericShader[2]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
        case GSTest:
            setShader(mGenericShader[3]);
            setShaderConstBuffer( mGenericShaderConst[3] );
            mGenericShaderConst[3]->setSafe( mGenericShader[3]->getShaderConstHandle("$mvp_matrix"), xform );
            break;
        case GSAlphaTexture:
            setShader(mGenericShader[4]);
            setShaderConstBuffer( mGenericShaderConst[4] );
            mGenericShaderConst[4]->setSafe( mGenericShader[4]->getShaderConstHandle("$mvp_matrix"), xform );
            mGenericShaderConst[4]->setSafe( mGenericShader[4]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
            //        case GSTargetRestore:
            
        default:
            break;
    }
}

GFXOpenGL32Shader* GFXOpenGL32Device::createShader()
{
    GFXOpenGL32Shader* shader = new GFXOpenGL32Shader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGL32Device::setShader( GFXOpenGL32Shader *shader )
{
    if ( shader )
    {
        if (shader != mpCurrentShader)
        {
            mpCurrentShader = shader;
            shader->useProgram();
        }
    }
    else
    {
        mpCurrentShader = NULL;
        glUseProgram(0);
    }
}

void GFXOpenGL32Device::disableShaders()
{
    setShader(NULL);
    setShaderConstBuffer( NULL );
}

void GFXOpenGL32Device::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGL32ShaderConstBuffer*>(buffer)->activate();
}

U32 GFXOpenGL32Device::getNumSamplers() const
{
    return mMaxShaderTextures;
}

U32 GFXOpenGL32Device::getNumRenderTargets() const
{
    return 1;
}


void GFXOpenGL32Device::_updateRenderTargets()
{
    if ( mRTDirty || mCurrentRT->isPendingState() )
    {
        if ( mRTDeactivate )
        {
            mRTDeactivate->deactivate();
            mRTDeactivate = NULL;
        }
        
        // NOTE: The render target changes is not really accurate
        // as the GFXTextureTarget supports MRT internally.  So when
        // we activate a GFXTarget it could result in multiple calls
        // to SetRenderTarget on the actual device.
        //      mDeviceStatistics.mRenderTargetChanges++;
        
        GFXOpenGL32TextureTarget *tex = dynamic_cast<GFXOpenGL32TextureTarget*>( mCurrentRT.getPointer() );
        if ( tex )
        {
            tex->applyState();
            tex->makeActive();
        }
        else
        {
            GFXOpenGL32WindowTarget *win = dynamic_cast<GFXOpenGL32WindowTarget*>( mCurrentRT.getPointer() );
            AssertFatal( win != NULL,
                        "GFXOpenGL32Device::_updateRenderTargets() - invalid target subclass passed!" );
            
            GL_CHECK();
            win->makeActive();
            
            if( win->mContext != static_cast<GFXOpenGL32Device*>(GFX)->mContext )
            {
                mRTDirty = false;
                GFX->updateStates(true);
            }
        }
        
        mRTDirty = false;
    }
    
    if ( mViewportDirty )
    {
        glViewport( mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y );
        mViewportDirty = false;
    }
}


GFXFormat GFXOpenGL32Device::selectSupportedFormat(   GFXTextureProfile* profile, 
                                               const Vector<GFXFormat>& formats, 
                                               bool texture, 
                                               bool mustblend,
                                               bool mustfilter )
{
//    for(U32 i = 0; i < formats.size(); i++)
//    {
//        // Single channel textures are not supported by FBOs.
//        if(profile->testFlag(GFXTextureProfile::RenderTarget) && (formats[i] == GFXFormatA8 || formats[i] == GFXFormatL8 || formats[i] == GFXFormatL16))
//            continue;
//        if(GFXGLTextureInternalFormat[formats[i]] == GL_ZERO)
//            continue;
//        
//        return formats[i];
//    }
//    
    return GFXFormatR8G8B8A8;
}

//
// Register this device with GFXInit
//
class GFXOpenGLRegisterDevice
{
public:
    GFXOpenGLRegisterDevice()
    {
        GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGL32Device::enumerateAdapters);
    }
};

static GFXOpenGLRegisterDevice pGLRegisterDevice;

//-----------------------------------------------------------------------------


void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}