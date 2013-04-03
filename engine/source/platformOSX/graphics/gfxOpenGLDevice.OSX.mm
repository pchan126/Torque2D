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
#include "./gfxOpenGLDevice.h"

//#include "graphics/gfxCubemap.h"
#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxInit.h"

#include "./gfxOpenGLEnumTranslate.h"
#include "./gfxOpenGLVertexBuffer.h"
#include "./gfxOpenGLPrimitiveBuffer.h"
#include "./gfxOpenGLTextureTarget.h"
#include "./gfxOpenGLTextureManager.h"
#include "./gfxOpenGLTextureObject.h"
//#include "./gfxOpenGLCubemap.h"
#include "./gfxOpenGLCardProfiler.h"
#include "./gfxOpenGLWindowTarget.h"

#include "./gfxOpenGLShader.h"
#include "graphics/primBuilder.h"
#include "console/console.h"

//#include "./gfxOpenGLOcclusionQuery.h"

GFXAdapter::CreateDeviceInstanceDelegate GFXOpenGLDevice::mCreateDeviceInstance(GFXOpenGLDevice::createInstance);

GFXDevice *GFXOpenGLDevice::createInstance( U32 adapterIndex )
{
    return new GFXOpenGLDevice(adapterIndex);
}

#include "osxGLUtils.h"

void GFXOpenGLDevice::initGLState()
{
    // Currently targeting OpenGL 3.2 (Mac)
    
    // We don't currently need to sync device state with a known good place because we are
    // going to set everything in GFXOpenGLStateBlock, but if we change our GFXOpenGLStateBlock strategy, this may
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
GFXOpenGLDevice::GFXOpenGLDevice( U32 adapterIndex ) :
                        mAdapterIndex(adapterIndex),
                        mCurrentVB(NULL),
                        mCurrentPB(NULL),
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
    
    mDeviceName = "OpenGL";
    mFullScreenOnly = false;
}


GFXOpenGLDevice::~GFXOpenGLDevice()
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


void GFXOpenGLDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    if(!mInitialized)
    {
       AssertFatal(!mContext && !mPixelFormat, "_createInitialContextAndFormat - Already created initial context and format");
       
       mPixelFormat = generateValidPixelFormat(mode.fullScreen, mode.bitDepth, 0);
       AssertFatal(mPixelFormat, "_createInitialContextAndFormat - Unable to create an OpenGL pixel format");
       
       mContext = [[[NSOpenGLContext alloc] initWithFormat: (NSOpenGLPixelFormat*)mPixelFormat shareContext: nil] retain];
       AssertFatal(mContext, "_createInitialContextAndFormat - Unable to create an OpenGL context");

       [mContext makeCurrentContext];
       
       mTextureManager = new GFXOpenGLTextureManager(mContext);

       initGLState();
       initGenericShaders();
       mInitialized = true;
       deviceInited();
    }
}

void GFXOpenGLDevice::addVideoMode(GFXVideoMode toAdd)
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

void GFXOpenGLDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
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

void GFXOpenGLDevice::enumerateVideoModes()
{
    mVideoModes.clear();
    CGDirectDisplayID display = CGMainDisplayID();
    
    // Enumerate all available resolutions:
    CFArrayRef modeArray = CGDisplayCopyAllDisplayModes( display, NULL );
    CFArrayApplyFunction(modeArray, CFRangeMake(0,CFArrayGetCount(modeArray)), addVideoModeCallback, &mVideoModes);
}


bool GFXOpenGLDevice::beginSceneInternal()
{
    // Nothing to do here for GL.
    mCanCurrentlyRender = true;
    return true;
}


inline void GFXOpenGLDevice::pushWorldMatrix()
{
    mWorldMatrixDirty = true;
    mStateDirty = true;
    
    MatrixF newMatrix = m_WorldStack.last();
    m_WorldStack.push_back(newMatrix);
//    GLKMatrixStackPush(m_WorldStackRef);
}

inline void GFXOpenGLDevice::popWorldMatrix()
{
    mWorldMatrixDirty = true;
    mStateDirty = true;
    
    m_WorldStack.pop_back();
//    GLKMatrixStackPop(m_WorldStackRef);
}

inline void GFXOpenGLDevice::multWorld( const MatrixF &mat )
{
    mWorldMatrixDirty = true;
    mStateDirty = true;
    
    MatrixF newMatrix = m_WorldStack.last();
    newMatrix*=mat;
    m_WorldStack.last() = newMatrix;
//    GLKMatrixStackMultiplyMatrix4(m_WorldStackRef, GLKMatrix4MakeWithArray(mat));
}

//inline void GFXOpenGLDevice::setTextureMatrix( const U32 stage, const MatrixF &texMat )
//{
//    AssertFatal( stage < TEXTURE_STAGE_COUNT, "Out of range texture sampler" );
//    mStateDirty = true;
//    mTextureMatrixDirty[stage] = true;
//    mTextureMatrix[stage] = texMat;
//    mTextureMatrixCheckDirty = true;
//}



void GFXOpenGLDevice::zombify()
{
    mTextureManager->zombify();
    if(mCurrentVB)
        mCurrentVB->finish();
    if(mCurrentPB)
        mCurrentPB->finish();
    //mVolatileVBs.clear();
    //mVolatilePBs.clear();
    GFXResource* walk = mResourceListHead;
    while(walk)
    {
        walk->zombify();
        walk = walk->getNextResource();
    }
}

void GFXOpenGLDevice::resurrect()
{
    GFXResource* walk = mResourceListHead;
    while(walk)
    {
        walk->resurrect();
        walk = walk->getNextResource();
    }
    if(mCurrentVB)
        mCurrentVB->prepare();
    if(mCurrentPB)
        mCurrentPB->prepare();
    mTextureManager->resurrect();
}


GFXVertexBuffer* GFXOpenGLDevice::findVolatileVBO(U32 numVerts, const GFXVertexFormat *vertexFormat, U32 vertSize, void* data)
{
    for(U32 i = 0; i < mVolatileVBs.size(); i++)
        if (  mVolatileVBs[i]->mNumVerts >= numVerts &&
            mVolatileVBs[i]->mVertexFormat.isEqual( *vertexFormat ) &&
            mVolatileVBs[i]->mVertexSize == vertSize &&
            mVolatileVBs[i]->getRefCount() == 1 )
        {
            mVolatileVBs[i].getPointer()->set(data, numVerts*vertSize);
            return mVolatileVBs[i];
        }
    
    // No existing VB, so create one
    StrongRefPtr<GFXOpenGLVertexBuffer> buf(new GFXOpenGLVertexBuffer(GFX, numVerts, vertexFormat, vertSize, GFXBufferTypeVolatile, data));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}

GFXPrimitiveBuffer* GFXOpenGLDevice::findVolatilePBO(U32 numIndices, U32 numPrimitives, U16* indexBuffer, GFXPrimitive *primitiveBuffer)
{
    for(U32 i = 0; i < mVolatilePBs.size(); i++)
        if((mVolatilePBs[i]->mIndexCount >= numIndices) && (mVolatilePBs[i]->getRefCount() == 1))
            return mVolatilePBs[i];
    
    // No existing PB, so create one
    StrongRefPtr<GFXOpenGLPrimitiveBuffer> buf(new GFXOpenGLPrimitiveBuffer(GFX, numIndices, numPrimitives, GFXBufferTypeVolatile, indexBuffer, primitiveBuffer));
    buf->registerResourceWithDevice(this);
    mVolatilePBs.push_back(buf);
    return buf.getPointer();
}

GFXVertexBuffer *GFXOpenGLDevice::allocVertexBuffer(   U32 numVerts,
                                                  const GFXVertexFormat *vertexFormat,
                                                  U32 vertSize,
                                                  GFXBufferType bufferType,
                                                  void *data)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(numVerts, vertexFormat, vertSize, data);
    
    GFXOpenGLVertexBuffer* buf = new GFXOpenGLVertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType, data );
    buf->registerResourceWithDevice(this);
    return buf;
}

GFXPrimitiveBuffer *GFXOpenGLDevice::allocPrimitiveBuffer( U32 numIndices, U32 numPrimitives, GFXBufferType bufferType, U16* indexBuffer, GFXPrimitive *primitiveBuffer )
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatilePBO(numIndices, numPrimitives, indexBuffer, primitiveBuffer);
    
    GFXOpenGLPrimitiveBuffer* buf = new GFXOpenGLPrimitiveBuffer(GFX, numIndices, numPrimitives, bufferType, indexBuffer, primitiveBuffer);
    buf->registerResourceWithDevice(this);
    return buf;
}

void GFXOpenGLDevice::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
    if (stream > 0) return;
    
    AssertFatal( stream == 0, "GFXOpenGLDevice::setVertexStream - We don't support multiple vertex streams!" );
    
    // Reset the state the old VB required, then set the state the new VB requires.
    if ( mCurrentVB )
        mCurrentVB->finish();
    
    mCurrentVB = static_cast<GFXOpenGLVertexBuffer*>( buffer );
    if ( mCurrentVB )
        mCurrentVB->prepare();
}

void GFXOpenGLDevice::setVertexStreamFrequency( U32 stream, U32 frequency )
{
    // We don't support vertex stream frequency or mesh instancing in OGL yet.
}

//GFXCubemap* GFXOpenGLDevice::createCubemap()
//{
////    GFXOpenGLCubemap* cube = new GFXOpenGLCubemap();
////    cube->registerResourceWithDevice(this);
////    return cube;
//};

void GFXOpenGLDevice::endSceneInternal()
{
    // nothing to do for opengl
    mCanCurrentlyRender = false;
}

void GFXOpenGLDevice::clear(U32 flags, ColorI color, F32 z, U32 stencil)
{
    GL_CHECK();
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


// Given a primitive type and a number of primitives, return the number of indexes/vertexes used.
GLsizei GFXOpenGLDevice::primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount)
{
    switch (primType)
    {
        case GFXPointList :
            return primitiveCount;
            break;
        case GFXLineList :
            return primitiveCount * 2;
            break;
        case GFXLineStrip :
            return primitiveCount + 1;
            break;
        case GFXTriangleList :
            return primitiveCount * 3;
            break;
        case GFXTriangleStrip :
            return 2 + primitiveCount;
            break;
        case GFXTriangleFan :
            return 2 + primitiveCount;
            break;
        default:
            AssertFatal(false, "GFXOpenGLDevice::primCountToIndexCount - unrecognized prim type");
            break;
    }
    
    return 0;
}

void GFXOpenGLDevice::updateStates(bool forceSetAll /*=false*/)
{
    GL_CHECK();
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
            setVertexStreamFrequency( i, mVertexBufferFrequency[i] );
        }
        
        if( mCurrentPrimitiveBuffer.isValid() ) // This could be NULL when the device is initalizing
            mCurrentPrimitiveBuffer->prepare();
        
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
    
    // Update primitive buffer
    //
    // NOTE: It is very important to set the primitive buffer AFTER the vertex buffer
    // because in order to draw indexed primitives in DX8, the call to SetIndicies
    // needs to include the base vertex offset, and the DX8 GFXDevice relies on
    // having mCurrentVB properly assigned before the call to setIndices -patw
    if( mPrimitiveBufferDirty )
    {
        if( mCurrentPrimitiveBuffer.isValid() ) // This could be NULL when the device is initalizing
            mCurrentPrimitiveBuffer->prepare();
        mPrimitiveBufferDirty = false;
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

inline void GFXOpenGLDevice::preDrawPrimitive()
{
    if( mStateDirty )
    {
        updateStates();
    }
    
    if(mCurrentShaderConstBuffer)
        setShaderConstBufferInternal(mCurrentShaderConstBuffer);
}

inline void GFXOpenGLDevice::postDrawPrimitive(U32 primitiveCount)
{
    //   mDeviceStatistics.mDrawCalls++;
    //   mDeviceStatistics.mPolyCount += primitiveCount;
}

void GFXOpenGLDevice::drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount )
{
    preDrawPrimitive();
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);

    
    // There are some odd performance issues if a buffer is bound to GL_ELEMENT_ARRAY_BUFFER when glDrawArrays is called.  Unbinding the buffer
    // improves performance by 10%.
    if(mCurrentPB)
        mCurrentPB->finish();
    
//    switch (GFXGLPrimType[primType])
//    {
//        case GL_TRIANGLE_STRIP:
//            Con::printf("GL_TRIANGLE_STRIP, %i %i", vertexStart, primCountToIndexCount(primType, primitiveCount));
//            break;
//            
//        case GL_LINE_STRIP:
//            Con::printf("GL_LINE_STRIP, %i %i", vertexStart, primCountToIndexCount(primType, primitiveCount));
//            break;
//
//        case GL_POINTS:
//            Con::printf("GL_POINTS, %i %i", vertexStart, primCountToIndexCount(primType, primitiveCount));
//            break;
//
//        default:
//            Con::printf("UNKNOWN TYPE");
//            break;
//    }
    
//    Con::printf(GFXGLPrimType[primType])
    glDrawArrays(GFXGLPrimType[primType], vertexStart, primCountToIndexCount(primType, primitiveCount));
    
    if(mCurrentPB)
        mCurrentPB->prepare();
    
    postDrawPrimitive(primitiveCount);
}

void GFXOpenGLDevice::drawIndexedPrimitive(   GFXPrimitiveType primType,
                                         U32 startVertex,
                                         U32 minIndex,
                                         U32 numVerts,
                                         U32 startIndex,
                                         U32 primitiveCount )
{
    AssertFatal( startVertex == 0, "GFXOpenGLDevice::drawIndexedPrimitive() - Non-zero startVertex unsupported!" );
    
    preDrawPrimitive();
    
    U16* buf = (U16*)static_cast<GFXOpenGLPrimitiveBuffer*>(mCurrentPrimitiveBuffer.getPointer())->getBuffer() + startIndex;
    
    glDrawElements(GFXGLPrimType[primType], primCountToIndexCount(primType, primitiveCount), GL_UNSIGNED_SHORT, buf);
    
    postDrawPrimitive(primitiveCount);
}

void GFXOpenGLDevice::setPB(GFXOpenGLPrimitiveBuffer* pb)
{
    if(mCurrentPB)
        mCurrentPB->finish();
    mCurrentPB = pb;
}

void GFXOpenGLDevice::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
    //   if(!lightEnable)
    //   {
    //      glDisable(GL_LIGHT0 + lightStage);
    //      return;
    //   }
    //
    //   if(light.mType == GFXLightInfo::Ambient)
    //   {
    //      AssertFatal(false, "Instead of setting an ambient light you should set the global ambient color.");
    //      return;
    //   }
    //
    //   GLenum lightEnum = GL_LIGHT0 + lightStage;
    //   glLightfv(lightEnum, GL_AMBIENT, (GLfloat*)&light.mAmbient);
    //   glLightfv(lightEnum, GL_DIFFUSE, (GLfloat*)&light.mColor);
    //   glLightfv(lightEnum, GL_SPECULAR, (GLfloat*)&light.mColor);
    //
    //   F32 pos[4];
    //
    //   if(light.mType != GFXLightInfo::Vector)
    //   {
    //      dMemcpy(pos, &light.mPos, sizeof(light.mPos));
    //      pos[3] = 1.0;
    //   }
    //   else
    //   {
    //      dMemcpy(pos, &light.mDirection, sizeof(light.mDirection));
    //      pos[3] = 0.0;
    //   }
    //   // Harcoded attenuation
    //   glLightf(lightEnum, GL_CONSTANT_ATTENUATION, 1.0f);
    //   glLightf(lightEnum, GL_LINEAR_ATTENUATION, 0.1f);
    //   glLightf(lightEnum, GL_QUADRATIC_ATTENUATION, 0.0f);
    //
    //   glLightfv(lightEnum, GL_POSITION, (GLfloat*)&pos);
    //   glEnable(lightEnum);
}

void GFXOpenGLDevice::setLightMaterialInternal(const GFXLightMaterial mat)
{
    //   // CodeReview - Setting these for front and back is unnecessary.  We should consider
    //   // checking what faces we're culling and setting this only for the unculled faces.
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat*)&mat.ambient);
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&mat.diffuse);
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat*)&mat.specular);
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (GLfloat*)&mat.emissive);
    //   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess);
}

void GFXOpenGLDevice::setGlobalAmbientInternal(ColorF color)
{
    //   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat*)&color);
}

void GFXOpenGLDevice::setTextureInternal(U32 textureUnit, const GFXTextureObject *texture)
{
    const GFXOpenGLTextureObject *tex = static_cast<const GFXOpenGLTextureObject*>(texture);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    if (tex)
    {
        // GFXOpenGLTextureObject::bind also handles applying the current sampler state.
        if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
            GL_CHECK(glBindTexture(mActiveTextureType[textureUnit], 0));
        mActiveTextureType[textureUnit] = tex->getBinding();
        tex->bind(textureUnit);
    }
    else if(mActiveTextureType[textureUnit] != GL_ZERO)
    {
        GL_CHECK(glBindTexture(mActiveTextureType[textureUnit], 0));
        mActiveTextureType[textureUnit] = GL_ZERO;
    }
    
    glActiveTexture(GL_TEXTURE0);
}

//void GFXOpenGLDevice::setCubemapInternal(U32 textureUnit, const GFXOpenGLCubemap* texture)
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

void GFXOpenGLDevice::setMatrix( GFXMatrixType mtype, const MatrixF &mat )
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
            AssertFatal(false, "GFXOpenGLDevice::setMatrix - Unknown matrix mode!");
            return;
    }
}


const MatrixF GFXOpenGLDevice::getMatrix( GFXMatrixType mtype )
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
            AssertFatal(false, "GFXOpenGLDevice::setMatrix - Unknown matrix mode!");
    }
    return ret;
}

void GFXOpenGLDevice::setClipRect( const RectI &inRect )
{
    AssertFatal(mCurrentRT.isValid(), "GFXOpenGLDevice::setClipRect - must have a render target set to do any rendering operations!");
    
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
GFXStateBlockRef GFXOpenGLDevice::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
    return GFXStateBlockRef(new GFXOpenGLStateBlock(desc));
}

/// Activates a stateblock
void GFXOpenGLDevice::setStateBlockInternal(GFXStateBlock* block, bool force)
{
    AssertFatal(dynamic_cast<GFXOpenGLStateBlock*>(block), "GFXOpenGLDevice::setStateBlockInternal - Incorrect stateblock type for this device!");
    GFXOpenGLStateBlock* glBlock = static_cast<GFXOpenGLStateBlock*>(block);
    GFXOpenGLStateBlock* glCurrent = static_cast<GFXOpenGLStateBlock*>(mCurrentStateBlock.getPointer());
    if (force)
        glCurrent = NULL;
    
    glBlock->activate(glCurrent); // Doesn't use current yet.
    mCurrentGLStateBlock = glBlock;
}

////------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GFXWindowTarget *GFXOpenGLDevice::allocWindowTarget(PlatformWindow *window)
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
    GFXOpenGLWindowTarget *gwt = new GFXOpenGLWindowTarget(window, this);
    gwt->mContext = ctx ? ctx : mContext;
    return gwt;
}


GFXTextureTarget * GFXOpenGLDevice::allocRenderToTextureTarget()
{
    GFXOpenGLTextureTarget *targ = new GFXOpenGLTextureTarget();
    targ->registerResourceWithDevice(this);
    return targ;
}

//GFXOcclusionQuery* GFXOpenGLDevice::createOcclusionQuery()
//{
//    GFXOcclusionQuery *query = new GFXOpenGLOcclusionQuery( this );
//    query->registerResourceWithDevice(this);
//    return query;
//}

void GFXOpenGLDevice::initGenericShaders()
{
    Vector<GFXShaderMacro> macros;
    char vertBuffer[1024];
    char fragBuffer[1024];
    //  #Color Shader
    
    const char* shaderDirectory = Con::getVariable("$GUI::shaderDirectory");
    Con::printf("loading shaders from %s", shaderDirectory);
    
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/CVert.glsl", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/CFrag.glsl", shaderDirectory);

    mGenericShader[0] = createShader();
    mGenericShader[0]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[0] = mGenericShader[0]->allocConstBuffer();
    
    //  #Texture Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/simpleVert.glsl", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/simpleFrag.glsl", shaderDirectory);
    
    mGenericShader[1] = createShader();
    mGenericShader[1]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[1] = mGenericShader[1]->allocConstBuffer();
    
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/pointVert.glsl", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/pointFrag.glsl", shaderDirectory);
    
    mGenericShader[2] = createShader();
    mGenericShader[2]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[2] = mGenericShader[2]->allocConstBuffer();
    
    //    GFXShaderConstHandle* hand = mGenericShader[0]->getShaderConstHandle("$mvp_matrix");
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/testVert.glsl", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/testFrag.glsl", shaderDirectory);
    
    mGenericShader[3] = createShader();
    mGenericShader[3]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[3] = mGenericShader[3]->allocConstBuffer();

    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/alphaVert.glsl", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/alphaFrag.glsl", shaderDirectory);
    
    mGenericShader[4] = createShader();
    mGenericShader[4]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[4] = mGenericShader[4]->allocConstBuffer();
}


void GFXOpenGLDevice::setupGenericShaders( GenericShaderType type )
{
//    GLKBaseEffect *GenericEffect = static_cast<GLKBaseEffect*>(baseEffect);
    
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
            GL_CHECK(setShader(mGenericShader[1]));
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
    
//    GLKBaseEffect *GenericEffect = static_cast<GLKBaseEffect*>(baseEffect);
//    
//    MatrixF worldViewMatrix(GFX->getViewMatrix());
//    worldViewMatrix *= GFX->getWorldMatrix();
//    worldViewMatrix.transpose();
//   
//   switch (type) {
//      case GSColor:
//         GenericEffect.texture2d0.enabled = false;
//         GenericEffect.texture2d1.enabled = false;
//         GenericEffect.transform.modelviewMatrix = worldViewMatrix.getMatrix();
//         GenericEffect.transform.projectionMatrix = GFX->getProjectionMatrix().getMatrix();
//         break;
//      case GSTexture:
//      case GSModColorTexture:
//      case GSAddColorTexture:
//      case GSPoint:
//         GenericEffect.transform.modelviewMatrix = worldViewMatrix.getMatrix();
//         GenericEffect.transform.projectionMatrix = GFX->getProjectionMatrix().getMatrix();
//         GenericEffect.texture2d0.enabled = true;
//         GenericEffect.texture2d1.enabled = false;
//         break;
//            
//        default:
//            break;
//    }
//   
//   [ GenericEffect prepareToDraw ];
}

GFXOpenGLShader* GFXOpenGLDevice::createShader()
{
    GFXOpenGLShader* shader = new GFXOpenGLShader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGLDevice::setShader( GFXOpenGLShader *shader )
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

void GFXOpenGLDevice::disableShaders()
{
    setShader(NULL);
    setShaderConstBuffer( NULL );
}

void GFXOpenGLDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGLShaderConstBuffer*>(buffer)->activate();
}

U32 GFXOpenGLDevice::getNumSamplers() const
{
    return mMaxShaderTextures;
}

U32 GFXOpenGLDevice::getNumRenderTargets() const
{
    return 1;
}


void GFXOpenGLDevice::_updateRenderTargets()
{
    GL_CHECK();
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
        
        GFXOpenGLTextureTarget *tex = dynamic_cast<GFXOpenGLTextureTarget*>( mCurrentRT.getPointer() );
        if ( tex )
        {
            tex->applyState();
            tex->makeActive();
        }
        else
        {
            GFXOpenGLWindowTarget *win = dynamic_cast<GFXOpenGLWindowTarget*>( mCurrentRT.getPointer() );
            AssertFatal( win != NULL,
                        "GFXOpenGLDevice::_updateRenderTargets() - invalid target subclass passed!" );
            
            GL_CHECK();
            win->makeActive();
            
            if( win->mContext != static_cast<GFXOpenGLDevice*>(GFX)->mContext )
            {
                mRTDirty = false;
                GFX->updateStates(true);
            }
        }
        
        mRTDirty = false;
    }
    
    if ( mViewportDirty )
    {
//        Con::printf("if mViewport Dirty %d %d %d %d", mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y);
        glViewport( mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y );
        mViewportDirty = false;
    }
}


GFXFormat GFXOpenGLDevice::selectSupportedFormat(   GFXTextureProfile* profile, 
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
//    return GFXFormatR8G8B8A8;
}

//
// Register this device with GFXInit
//
class GFXOpenGLRegisterDevice
{
public:
    GFXOpenGLRegisterDevice()
    {
        GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGLDevice::enumerateAdapters);
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