//
// Created by Paul L Jan on 2013-08-19.
//

#ifndef __ShaderAsset_H_
#define __ShaderAsset_H_

#include "assets/assetBase.h"
#include "collection/vector.h"
#include "2d/core/Vector2.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureHandle.h"
#include "graphics/gfxTextureManager.h"

//-----------------------------------------------------------------------------

DefineConsoleType( TypeShaderAssetPtr )
GFX_DeclareTextureProfile(GFXShaderAssetTextureProfile);

//-----------------------------------------------------------------------------

class ShaderAsset : public AssetBase
{
private:
    typedef AssetBase Parent;

protected:
    /// Configuration.
    StringTableEntry            mVertexShaderFile;
    StringTableEntry            mFragmentShaderFile;
    GFXShaderRef                mShader;
    GFXShaderConstBufferRef     mShaderConst;

public:
    ShaderAsset();
    virtual ~ShaderAsset();

    /// Core.
    static void initPersistFields();
    virtual bool onAdd();
    virtual void onRemove();
    virtual void copyTo(SimObject* object);

    void                    setVertexShaderFile( const char* pVertexShaderFile );
    inline StringTableEntry getVertexShaderFile( void ) const                      { return mVertexShaderFile; };

    void                    setFragmentShaderFile( const char* pFragmentShaderFile );
    inline StringTableEntry getFragmentShaderFile( void ) const                      { return mFragmentShaderFile; };

    inline GFXShaderRef                   getShader( void )                         { return mShader; }
    inline GFXShaderConstBufferRef&       getShaderConstBuffer( void )              { return mShaderConst; }

    /// Declare Console Object.
    DECLARE_CONOBJECT(ShaderAsset);

protected:
    virtual void initializeAsset( void );
    virtual void onAssetRefresh( void );

    /// Taml callbacks.
    virtual void onTamlPreWrite( void );
    virtual void onTamlPostWrite( void );
    virtual void onTamlCustomWrite( TamlCustomNodes& customNodes );
    virtual void onTamlCustomRead( const TamlCustomNodes& customNodes );

    void compileShader();
protected:
    static bool setVertexShaderFile( void* obj, const char* data )                 { static_cast<ShaderAsset*>(obj)->setVertexShaderFile(data); return false; }
    static const char* getVertexShaderFile(void* obj, const char* data)            { return static_cast<ShaderAsset*>(obj)->getVertexShaderFile(); }
    static bool writeVertexShaderFile( void* obj, StringTableEntry pFieldName )    { return static_cast<ShaderAsset*>(obj)->getVertexShaderFile() != StringTable->EmptyString; }

    static bool setFragmentShaderFile( void* obj, const char* data )                 { static_cast<ShaderAsset*>(obj)->setFragmentShaderFile(data); return false; }
    static const char* getFragmentShaderFile(void* obj, const char* data)            { return static_cast<ShaderAsset*>(obj)->getFragmentShaderFile(); }
    static bool writeFragmentShaderFile( void* obj, StringTableEntry pFieldName )    { return static_cast<ShaderAsset*>(obj)->getFragmentShaderFile() != StringTable->EmptyString; }
};



#endif //__ShaderAsset_H_
