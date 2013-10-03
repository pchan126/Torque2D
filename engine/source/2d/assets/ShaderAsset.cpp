//
// Created by Paul L Jan on 2013-08-19.
//

#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/consoleTypes.h"
#include "platform/platform.h"
#include "2d/core/Utility.h"
#include "2d/sceneobject/SceneObject.h"
#include "2d/assets/ShaderAsset.h"

// Debug Profiling.
#include "debug/profiler.h"

//------------------------------------------------------------------------------

ConsoleType( shaderAssetPtr, TypeShaderAssetPtr, sizeof(AssetPtr<ShaderAsset>), ASSET_ID_FIELD_PREFIX )

//-----------------------------------------------------------------------------

ConsoleGetType( TypeShaderAssetPtr )
{
    // Fetch asset Id.
    return (*((AssetPtr<ShaderAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType( TypeShaderAssetPtr )
{
    // Was a single argument specified?
    if( argc == 1 )
    {
        // Yes, so fetch field value.
        const char* pFieldValue = argv[0];

        // Fetch asset pointer.
        AssetPtr<ShaderAsset>* pAssetPtr = dynamic_cast<AssetPtr<ShaderAsset>*>((AssetPtrBase*)(dptr));

        // Is the asset pointer the correct type?
        if ( pAssetPtr == nullptr )
        {
            // No, so fail.
            Con::warnf( "(TypeShaderAssetPtr) - Failed to set asset Id '%d'.", pFieldValue );
            return;
        }

        // Set asset.
        pAssetPtr->setAssetId( pFieldValue );

        return;
    }

    // Warn.
    Con::warnf( "(TypeShaderAssetPtr) - Cannot set multiple args to a single asset." );
}

ShaderAsset::ShaderAsset() :  mVertexShaderFile(StringTable->EmptyString),
                              mFragmentShaderFile(StringTable->EmptyString)
{
}

//------------------------------------------------------------------------------

ShaderAsset::~ShaderAsset()
{
   SimObject::onRemove();
}


//------------------------------------------------------------------------------

void ShaderAsset::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    // Fields.
    addProtectedField("VertexShaderFile", TypeAssetLooseFilePath, Offset(mVertexShaderFile, ShaderAsset), &setVertexShaderFile, &getVertexShaderFile, &defaultProtectedWriteFn, "");
    addProtectedField("FragmentShaderFile", TypeAssetLooseFilePath, Offset(mFragmentShaderFile, ShaderAsset), &setFragmentShaderFile, &getFragmentShaderFile, &defaultProtectedWriteFn, "");
}

//------------------------------------------------------------------------------

bool ShaderAsset::onAdd()
{
    // Call Parent.
    if (!Parent::onAdd())
        return false;

    return true;
}

//------------------------------------------------------------------------------

void ShaderAsset::onRemove()
{
    // Call Parent.
    Parent::onRemove();
}

//------------------------------------------------------------------------------

void ShaderAsset::copyTo(SimObject* object)
{
    // Call to parent.
    Parent::copyTo(object);

    // Cast to asset.
    ShaderAsset* pAsset = static_cast<ShaderAsset*>(object);

    // Sanity!
    AssertFatal(pAsset != nullptr, "ShaderAsset::copyTo() - Object is not the correct type.");

    // Copy state.
    pAsset->setVertexShaderFile(getVertexShaderFile() );
    pAsset->setFragmentShaderFile(getFragmentShaderFile() );
}

//------------------------------------------------------------------------------

void ShaderAsset::setVertexShaderFile( const char* pVertexShaderFile )
{
    // Sanity!
    AssertFatal( pVertexShaderFile != nullptr, "Cannot use a NULL shader file." );

    // Fetch image file.
    pVertexShaderFile = StringTable->insert( pVertexShaderFile );

    // Ignore no change,
    if ( pVertexShaderFile == mVertexShaderFile )
        return;

    // Update.
    mVertexShaderFile = getOwned() ? expandAssetFilePath( pVertexShaderFile ) : StringTable->insert( pVertexShaderFile );

    // Refresh the asset.
    refreshAsset();
}

//------------------------------------------------------------------------------

void ShaderAsset::setFragmentShaderFile( const char* pFragmentShaderFile )
{
    // Sanity!
    AssertFatal( pFragmentShaderFile != nullptr, "Cannot use a NULL shader file." );

    // Fetch image file.
    pFragmentShaderFile = StringTable->insert( pFragmentShaderFile );

    // Ignore no change,
    if ( pFragmentShaderFile == mFragmentShaderFile )
        return;

    // Update.
    mFragmentShaderFile = getOwned() ? expandAssetFilePath( pFragmentShaderFile ) : StringTable->insert( pFragmentShaderFile );

    // Refresh the asset.
    refreshAsset();
}

//------------------------------------------------------------------------------

void ShaderAsset::onAssetRefresh( void )
{
    // Ignore if not yet added to the sim.
    if ( !isProperlyAdded() )
        return;

    // Call parent.
    Parent::onAssetRefresh();

    // Compile shader.
    compileShader();
}

//-----------------------------------------------------------------------------

void ShaderAsset::onTamlPreWrite( void )
{
    // Call parent.
    Parent::onTamlPreWrite();

    // Ensure the shader-file is collapsed.
    mVertexShaderFile = collapseAssetFilePath( mVertexShaderFile );
    mFragmentShaderFile = collapseAssetFilePath( mFragmentShaderFile );
}

//-----------------------------------------------------------------------------

void ShaderAsset::onTamlPostWrite( void )
{
    // Call parent.
    Parent::onTamlPostWrite();

    // Ensure the shader-file is expanded.
    mVertexShaderFile = expandAssetFilePath( mVertexShaderFile );
    mFragmentShaderFile = expandAssetFilePath( mFragmentShaderFile );
}

//------------------------------------------------------------------------------

void ShaderAsset::compileShader( void )
{
    Vector<GFXShaderMacro> macros;
    mShader = GFX->createShader();
    mShader->init(String(mVertexShaderFile), String(mFragmentShaderFile), 0, macros);
    mShaderConst = mShader->allocConstBuffer();
}


//------------------------------------------------------------------------------

void ShaderAsset::initializeAsset( void )
{
    // Call parent.
    Parent::initializeAsset();

    // Ensure the image-file is expanded.
    mVertexShaderFile = expandAssetFilePath( mVertexShaderFile );
    mFragmentShaderFile = expandAssetFilePath( mFragmentShaderFile );

    // Calculate the image.
    compileShader();
}


//------------------------------------------------------------------------------

void ShaderAsset::onTamlCustomWrite( TamlCustomNodes& customNodes )
{
    // Debug Profiling.
    PROFILE_SCOPE(ShaderAsset_OnTamlCustomWrite);

    // Call parent.
    Parent::onTamlCustomWrite( customNodes );
}

//-----------------------------------------------------------------------------

void ShaderAsset::onTamlCustomRead( const TamlCustomNodes& customNodes )
{
    // Debug Profiling.
    PROFILE_SCOPE(ShaderAsset_OnTamlCustomRead);

    // Call parent.
    Parent::onTamlCustomRead( customNodes );
}


//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(ShaderAsset);
