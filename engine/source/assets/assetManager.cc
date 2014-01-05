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

#include "assetManager.h"
#include "assetPtr.h"
#include "assets/referencedAssets.h"
#include "assets/declaredAssets.h"
#include "tamlAssetReferencedVisitor.h"
#include "tamlAssetDeclaredVisitor.h"
#include "tamlAssetDeclaredUpdateVisitor.h"
#include "tamlAssetReferencedUpdateVisitor.h"
#include "console/consoleTypes.h"
#include "assetManager_ScriptBinding.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( AssetManager );

//-----------------------------------------------------------------------------

AssetManager AssetDatabase;

//-----------------------------------------------------------------------------

AssetManager::AssetManager() :
    mLoadedInternalAssetsCount( 0 ),
    mLoadedExternalAssetsCount( 0 ),
    mLoadedPrivateAssetsCount( 0 ),
    mMaxLoadedInternalAssetsCount( 0 ),
    mMaxLoadedExternalAssetsCount( 0 ),
    mMaxLoadedPrivateAssetsCount( 0 ),
    mAcquiredReferenceCount( 0 ),
    mEchoInfo( false ),
    mIgnoreAutoUnload( false )
{
}

//-----------------------------------------------------------------------------

bool AssetManager::onAdd()
{
    // Call parent.
    if ( !Parent::onAdd() )
        return false;

    return true;
}

//-----------------------------------------------------------------------------

void AssetManager::onRemove()
{
    // Do we have an asset tags manifest?
    if ( !mAssetTagsManifest.isNull() )
    {
        // Yes, so remove it.
        mAssetTagsManifest->deleteObject();
    }

    // Call parent.
    Parent::onRemove();
}

//-----------------------------------------------------------------------------

void AssetManager::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addField( "EchoInfo", TypeBool, Offset(mEchoInfo, AssetManager), "Whether the asset manager echos extra information to the console or not." );
    addField( "IgnoreAutoUnload", TypeBool, Offset(mIgnoreAutoUnload, AssetManager), "Whether the asset manager should ignore unloading of auto-unload assets or not." );
}

//-----------------------------------------------------------------------------

bool AssetManager::compileReferencedAssets( ModuleDefinition* pModuleDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_CompileReferencedAsset);

    // Sanity!
    AssertFatal( pModuleDefinition != nullptr, "Cannot add declared assets using a nullptr module definition" );
   assert(pModuleDefinition != nullptr);

    // Clear referenced assets.
    mReferencedAssets.clear();

    // Iterate the module definition children.
    for( auto itr : *pModuleDefinition )
    {
        // Fetch the referenced assets.
        ReferencedAssets* pReferencedAssets = dynamic_cast<ReferencedAssets*>( itr );

        // Skip if it's not a referenced assets location.
        if ( pReferencedAssets == nullptr )
            continue;

        // Expand asset manifest location.
        char filePathBuffer[1024];
        dSprintf( filePathBuffer, sizeof(filePathBuffer), "%s/%s", pModuleDefinition->getModulePath(), pReferencedAssets->getPath() );

        // Scan referenced assets at location.
        if ( !scanReferencedAssets( filePathBuffer, pReferencedAssets->getExtension(), pReferencedAssets->getRecurse() ) )
        {
            // Warn.
            Con::warnf( "AssetManager::compileReferencedAssets() - Could not scan for referenced assets at location '%s' with extension '%s'.", filePathBuffer, pReferencedAssets->getExtension() );
        }
    }  

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::addModuleDeclaredAssets( ModuleDefinition* pModuleDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_AddDeclaredAssets);

    // Sanity!
    AssertFatal( pModuleDefinition != nullptr, "Cannot add declared assets using a nullptr module definition" );
   assert(pModuleDefinition != nullptr);

    // Does the module have any assets associated with it?
    if ( pModuleDefinition->getModuleAssets().size() > 0 )
    {
        // Yes, so warn.
        Con::warnf( "Asset Manager: Cannot add declared assets to module '%s' as it already has existing assets.", pModuleDefinition->getSignature() );
        return false;
    }

    // Iterate the module definition children.
    for( auto itr : *pModuleDefinition )
    {
        // Fetch the declared assets.
        DeclaredAssets* pDeclaredAssets = dynamic_cast<DeclaredAssets*>( itr );

        // Skip if it's not a declared assets location.
        if ( pDeclaredAssets == nullptr )
            continue;

        // Expand asset manifest location.
        char filePathBuffer[1024];
        dSprintf( filePathBuffer, sizeof(filePathBuffer), "%s/%s", pModuleDefinition->getModulePath(), pDeclaredAssets->getPath() );

        // Scan declared assets at location.
        if ( !scanDeclaredAssets( filePathBuffer, pDeclaredAssets->getExtension(), pDeclaredAssets->getRecurse(), pModuleDefinition ) )
        {
            // Warn.
            Con::warnf( "AssetManager::addModuleDeclaredAssets() - Could not scan for declared assets at location '%s' with extension '%s'.", filePathBuffer, pDeclaredAssets->getExtension() );
        }
    }  

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::addDeclaredAsset( ModuleDefinition* pModuleDefinition, const char* pAssetFilePath )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_AddSingleDeclaredAsset);

    // Sanity!
    AssertFatal( pModuleDefinition != nullptr, "Cannot add single declared asset using a nullptr module definition" );
    AssertFatal( pAssetFilePath != nullptr, "Cannot add single declared asset using a nullptr asset file-path." );
   assert(pModuleDefinition != nullptr);
   assert(pAssetFilePath != nullptr);

    // Expand asset file-path.
    char assetFilePathBuffer[1024];
    Con::expandPath( assetFilePathBuffer, sizeof(assetFilePathBuffer), pAssetFilePath );

    // Find the final slash which should be just before the file.
    char* pFileStart = dStrrchr( assetFilePathBuffer, '/' );

    // Did we find the final slash?
    if ( pFileStart == nullptr )
    {
        // No, so warn.
        Con::warnf( "AssetManager::addDeclaredAsset() - Could not add single declared asset file '%s' as file-path '%s' is not valid.",
            assetFilePathBuffer,
            pModuleDefinition->getModulePath() );
        return false;
    }

    // Terminate path at slash.
    *pFileStart = 0;

    // Move to next character which should be the file start.
    pFileStart++;

    // Scan declared assets at location.
    if ( !scanDeclaredAssets( assetFilePathBuffer, pFileStart, false, pModuleDefinition ) )
    {
        // Warn.
        Con::warnf( "AssetManager::addDeclaredAsset() - Could not scan declared assets at location '%s' with extension '%s'.", assetFilePathBuffer, pFileStart );
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetManager::addPrivateAsset( AssetBase* pAssetBase )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_AddPrivateAsset);

    // Sanity!
    AssertFatal( pAssetBase != nullptr, "Cannot add a nullptr private asset." );
   assert(pAssetBase != nullptr);

    // Is the asset already added?
    if ( pAssetBase->mpAssetDefinition->mAssetId != StringTable->EmptyString )
    {
        // Yes, so warn.
        Con::warnf( "Cannot add private asset '%d' as it has already been assigned.", pAssetBase->mpAssetDefinition->mAssetId );
        return StringTable->EmptyString;
    }

    static U32 masterPrivateAssetId = 1;

    // Create asset definition.
    AssetDefinition* pAssetDefinition = new AssetDefinition();

    // Fetch source asset definition.
    AssetDefinition* pSourceAssetDefinition = pAssetBase->mpAssetDefinition;

    // Configure asset.
    pAssetDefinition->mpAssetBase = pAssetBase;
    pAssetDefinition->mAssetDescription = pSourceAssetDefinition->mAssetDescription;
    pAssetDefinition->mAssetCategory = pSourceAssetDefinition->mAssetCategory;
    pAssetDefinition->mAssetAutoUnload = true;
    pAssetDefinition->mAssetRefreshEnable = false;
    pAssetDefinition->mAssetType = StringTable->insert( pAssetBase->getClassName() );
    pAssetDefinition->mAssetLoadedCount = 0;
    pAssetDefinition->mAssetInternal = false;
    pAssetDefinition->mAssetPrivate = true;

    // Format asset name.
    char assetNameBuffer[256];
    dSprintf(assetNameBuffer, sizeof(assetNameBuffer), "%s_%d", pAssetDefinition->mAssetType, masterPrivateAssetId++ );    

    // Set asset identity.
    pAssetDefinition->mAssetName = StringTable->insert( assetNameBuffer );
    pAssetDefinition->mAssetId = pAssetDefinition->mAssetName;

    // Ensure that the source asset is fully synchronized with the new asset definition.
    pSourceAssetDefinition->mAssetName = pAssetDefinition->mAssetName;
    pSourceAssetDefinition->mAssetAutoUnload = pAssetDefinition->mAssetAutoUnload;
    pSourceAssetDefinition->mAssetInternal = pAssetDefinition->mAssetInternal;

    // Set ownership by asset manager.
    pAssetDefinition->mpAssetBase->setOwned( this, pAssetDefinition );

    // Store in declared assets.
    mDeclaredAssets.insert( pAssetDefinition->mAssetId, pAssetDefinition );

    // Increase the private loaded asset count.
    if ( ++mLoadedPrivateAssetsCount > mMaxLoadedPrivateAssetsCount )
        mMaxLoadedPrivateAssetsCount = mLoadedPrivateAssetsCount;

    return pAssetDefinition->mAssetId;
}

//-----------------------------------------------------------------------------

bool AssetManager::removeDeclaredAssets( ModuleDefinition* pModuleDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RemoveDeclaredAssets);

    // Sanity!
    AssertFatal( pModuleDefinition != nullptr, "Cannot remove declared assets using a nullptr module definition" );
   assert(pModuleDefinition != nullptr);

    // Fetch module assets.
    ModuleDefinition::typeModuleAssetsVector& moduleAssets = pModuleDefinition->getModuleAssets();

    // Remove all module assets.
    while ( moduleAssets.size() > 0 )
    {
        // Fetch asset definition.
        AssetDefinition* pAssetDefinition = moduleAssets.front();

        // Remove this asset.
        removeDeclaredAsset( pAssetDefinition->mAssetId );
    }

    // Info.
    if ( mEchoInfo )
        Con::printSeparator();

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::removeDeclaredAsset( const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RemoveSingleDeclaredAsset);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot remove single declared asset using nullptr asset Id." );
   assert(pAssetId != nullptr);

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Find declared asset.
    typeDeclaredAssetsHash::iterator declaredAssetItr = mDeclaredAssets.find( assetId );

    // Did we find the declared asset?
    if ( declaredAssetItr == mDeclaredAssets.end() )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot remove single asset Id '%s' it could not be found.", assetId );
        return false;
    }

    // Fetch asset definition.
    AssetDefinition* pAssetDefinition = declaredAssetItr->second;

    // Is the asset private?
    if ( !pAssetDefinition->mAssetPrivate )
    {
        // No, so fetch module assets.
        ModuleDefinition::typeModuleAssetsVector& moduleAssets = pAssetDefinition->mpModuleDefinition->getModuleAssets();

        // Remove module asset.
        auto moduleAssetItr = moduleAssets.find(pAssetDefinition );
        if ( moduleAssetItr != moduleAssets.end())
            moduleAssets.erase( moduleAssetItr );

        // Remove asset dependencies.
        removeAssetDependencies( pAssetId );
    }

    // Do we have an asset loaded?
    if ( pAssetDefinition->mpAssetBase.notNull() )
    {
        // Yes, so delete it.
        // NOTE: If anything is using this then this'll cause a crash.  Objects should always use safe reference methods however.
        pAssetDefinition->mpAssetBase->deleteObject();
    }

    // Remove from declared assets.
    mDeclaredAssets.erase( declaredAssetItr );

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Asset Manager: Removing Asset Id '%s' of type '%s' in asset file '%s'.",
            pAssetDefinition->mAssetId,
            pAssetDefinition->mAssetType,
            pAssetDefinition->mAssetBaseFilePath );
    }

    // Destroy asset definition.
    delete pAssetDefinition;

    return true;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetManager::getAssetName( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return nullptr;
    }

    return pAssetDefinition->mAssetName;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetManager::getAssetDescription( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return nullptr;
    }

    return pAssetDefinition->mAssetDescription;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetManager::getAssetCategory( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return nullptr;
    }

    return pAssetDefinition->mAssetCategory;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetManager::getAssetType( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return nullptr;
    }

    return pAssetDefinition->mAssetType;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetManager::getAssetFilePath( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return StringTable->EmptyString;
    }

    return pAssetDefinition->mAssetBaseFilePath;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetManager::getAssetPath( const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_GetAssetPath);

    // Fetch asset file-path.
    StringTableEntry assetFilePath = getAssetFilePath( pAssetId );

    // Finish if no file-path.
    if ( assetFilePath == StringTable->EmptyString  )
        return assetFilePath;

    // Find the final slash which should be just before the file.
    const char* pFinalSlash = dStrrchr( assetFilePath, '/' );

    // Sanity!
    AssertFatal( pFinalSlash != nullptr, "Should always be able to find final slash in the asset file-path." );

    // Fetch asset path.
    return StringTable->insertn( assetFilePath, pFinalSlash - assetFilePath );
}

//-----------------------------------------------------------------------------

ModuleDefinition* AssetManager::getAssetModuleDefinition( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return nullptr;
    }

    return pAssetDefinition->mpModuleDefinition;
}

//-----------------------------------------------------------------------------

bool AssetManager::isAssetInternal( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return false;
    }

    return pAssetDefinition->mAssetInternal;
}

//-----------------------------------------------------------------------------

bool AssetManager::isAssetPrivate( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return false;
    }

    return pAssetDefinition->mAssetPrivate;
}

//-----------------------------------------------------------------------------

bool AssetManager::isAssetAutoUnload( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return false;
    }

    return pAssetDefinition->mAssetAutoUnload;
}

//-----------------------------------------------------------------------------

bool AssetManager::isAssetLoaded( const char* pAssetId )
{
    // Find asset definition.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find asset Id '%s'.", pAssetId );
        return false;
    }

    return pAssetDefinition->mpAssetBase != nullptr;
}


//-----------------------------------------------------------------------------

bool AssetManager::isDeclaredAsset( const char* pAssetId )
{
    return findAsset( pAssetId ) != nullptr;
}

//-----------------------------------------------------------------------------

bool AssetManager::doesAssetDependOn( const char* pAssetId, const char* pDependsOnAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_DoesAssetDependOn);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot use nullptr asset Id." );
    AssertFatal( pDependsOnAssetId != nullptr, "Cannot use nullptr depends-on asset Id." );
   assert(pAssetId != nullptr);
   assert(pDependsOnAssetId != nullptr);

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Fetch depends-on asset Id.
    StringTableEntry dependsOnAssetId = StringTable->insert( pDependsOnAssetId );

    // Find depends-on entry.
    typeAssetDependsOnHash::iterator dependsOnItr = mAssetDependsOn.find( assetId );

    // Iterate all dependencies.
    while( dependsOnItr != mAssetDependsOn.end() && dependsOnItr->first == assetId )
    {
        // Finish if a depends on.
        if ( dependsOnItr->second == dependsOnAssetId )
            return true;

        // Next dependency.
        dependsOnItr++;
    }

    return false;
}

//-----------------------------------------------------------------------------

bool AssetManager::isAssetDependedOn( const char* pAssetId, const char* pDependedOnByAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_IsAssetDependedOn);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot use nullptr asset Id." );
    AssertFatal( pDependedOnByAssetId != nullptr, "Cannot use nullptr depended-on-by asset Id." );
   assert(pAssetId != nullptr);
   assert(pDependedOnByAssetId != nullptr);

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Fetch depended-on-by asset Id.
    StringTableEntry dependedOnByAssetId = StringTable->insert( pDependedOnByAssetId );

    // Find depended-on-by entry.
    typeAssetDependsOnHash::iterator dependedOnItr = mAssetIsDependedOn.find( assetId );

    // Iterate all dependencies.
    while( dependedOnItr != mAssetIsDependedOn.end() && dependedOnItr->first == assetId )
    {
        // Finish if depended-on.
        if ( dependedOnItr->second == dependedOnByAssetId )
            return true;

        // Next dependency.
        dependedOnItr++;
    }

    return false;
}

//-----------------------------------------------------------------------------

bool AssetManager::isReferencedAsset( const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_IsReferencedAsset);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot check if nullptr asset Id is referenced." );
   assert(pAssetId != nullptr);

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Is asset Id the correct format?
    if ( StringUnit::getUnitCount( assetId, ASSET_SCOPE_TOKEN ) != 2 )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot check if asset Id '%s' is referenced as it is not the correct format.", assetId );
        return false;
    }

    return mReferencedAssets.count( assetId ) > 0;
}

//-----------------------------------------------------------------------------

bool AssetManager::renameDeclaredAsset( const char* pAssetIdFrom, const char* pAssetIdTo )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RenameDeclaredAsset);

    // Sanity!
    AssertFatal( pAssetIdFrom != nullptr, "Cannot rename from nullptr asset Id." );
    AssertFatal( pAssetIdTo != nullptr, "Cannot rename to nullptr asset Id." );
   assert(pAssetIdTo != nullptr);
   assert(pAssetIdFrom != nullptr);

    // Fetch asset Ids.
    StringTableEntry assetIdFrom = StringTable->insert( pAssetIdFrom );
    StringTableEntry assetIdTo   = StringTable->insert( pAssetIdTo );

    // Is asset Id from the correct format?
    if ( StringUnit::getUnitCount( assetIdFrom, ASSET_SCOPE_TOKEN ) != 2 )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename declared asset Id '%s' to asset Id '%s' as source asset Id is not the correct format.", assetIdFrom, assetIdTo );
        return false;
    }

    // Is asset Id to the correct format?
    if ( StringUnit::getUnitCount( assetIdTo, ASSET_SCOPE_TOKEN ) != 2 )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename declared asset Id '%s' to asset Id '%s' as target asset Id is not the correct format.", assetIdFrom, assetIdTo );
        return false;
    }

    // Does the asset Id from exist?
    if ( !mDeclaredAssets.contains( assetIdFrom ) )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename declared asset Id '%s' to asset Id '%s' as source asset Id is not declared.", assetIdFrom, assetIdTo );
        return false;
    }

    // Does the asset Id to exist?
    if ( mDeclaredAssets.contains( assetIdTo ) )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename declared asset Id '%s' to asset Id '%s' as target asset Id is already declared.", assetIdFrom, assetIdTo );
        return false;
    }

    // Split module Ids from asset Ids.
    StringTableEntry moduleIdFrom = StringTable->insert( StringUnit::getUnit( assetIdFrom, 0, ASSET_SCOPE_TOKEN ) );
    StringTableEntry moduleIdTo   = StringTable->insert( StringUnit::getUnit( assetIdTo, 0, ASSET_SCOPE_TOKEN ) );

    // Are the module Ids the same?
    if ( moduleIdFrom != moduleIdTo )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename declared asset Id '%s' to asset Id '%s' as the module Id cannot be changed.", assetIdFrom, assetIdTo );
        return false;
    }

    // Find asset definition.
    typeDeclaredAssetsHash::iterator assetDefinitionItr =  mDeclaredAssets.find( assetIdFrom );

    // Sanity!
    AssertFatal( assetDefinitionItr != mDeclaredAssets.end(), "Asset Manager: Failed to find asset." );

    // Fetch asset definition.
    AssetDefinition* pAssetDefinition = assetDefinitionItr->second;

    // Is this a private asset?
    if ( pAssetDefinition->mAssetPrivate )
    {
        // Yes, so warn.
        Con::warnf("Asset Manager: Cannot rename declared asset Id '%s' to asset Id '%s' as the source asset is private.", assetIdFrom, assetIdTo );
        return false;
    }

    // Setup declared update visitor.
    TamlAssetDeclaredUpdateVisitor assetDeclaredUpdateVisitor;
    assetDeclaredUpdateVisitor.setAssetIdFrom( assetIdFrom );
    assetDeclaredUpdateVisitor.setAssetIdTo( assetIdTo );

    // Update asset file declaration.
    if ( !mTaml.parse( pAssetDefinition->mAssetBaseFilePath, assetDeclaredUpdateVisitor ) )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename declared asset Id '%s' to asset Id '%s' as the declared asset file could not be parsed: %s",
            assetIdFrom, assetIdTo, pAssetDefinition->mAssetBaseFilePath );
        return false;
    }

    // Update asset definition.
    pAssetDefinition->mAssetId = assetIdTo;
    pAssetDefinition->mAssetName = StringTable->insert( StringUnit::getUnit( assetIdTo, 1, ASSET_SCOPE_TOKEN ) );

    // Reinsert declared asset.
    mDeclaredAssets.erase( assetIdFrom );
    mDeclaredAssets.insert( assetIdTo, pAssetDefinition );

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Asset Manager: Renaming declared Asset Id '%s' to Asset Id '%s'.", assetIdFrom, assetIdTo );
        Con::printSeparator();
    }

    // Rename asset dependencies.
    renameAssetDependencies( assetIdFrom, assetIdTo );

    // Do we have an asset tags manifest?
    if ( !mAssetTagsManifest.isNull() )
    {
        // Yes, so rename any assets.
        mAssetTagsManifest->renameAssetId( pAssetIdFrom, pAssetIdTo );

        // Save the asset tags.
        saveAssetTags();
    }

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::renameReferencedAsset( const char* pAssetIdFrom, const char* pAssetIdTo )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RenameReferencedAsset);

    // Sanity!
    AssertFatal( pAssetIdFrom != nullptr, "Cannot rename from nullptr asset Id." );
    AssertFatal( pAssetIdTo != nullptr, "Cannot rename to nullptr asset Id." );
   assert(pAssetIdFrom != nullptr);
   assert(pAssetIdTo != nullptr);

    // Fetch asset Ids.
    StringTableEntry assetIdFrom = StringTable->insert( pAssetIdFrom );
    StringTableEntry assetIdTo   = StringTable->insert( pAssetIdTo );

    // Is asset Id from the correct format?
    if ( StringUnit::getUnitCount( assetIdFrom, ASSET_SCOPE_TOKEN ) != 2 )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename referenced asset Id '%s' to asset Id '%s' as source asset Id is not the correct format.", assetIdFrom, assetIdTo );
        return false;
    }

    // Is asset Id to the correct format?
    if ( StringUnit::getUnitCount( assetIdTo, ASSET_SCOPE_TOKEN ) != 2 )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename referenced asset Id '%s' to asset Id '%s' as target asset Id is not the correct format.", assetIdFrom, assetIdTo );
        return false;
    }

    // Does the asset Id to exist?
    if ( !mDeclaredAssets.contains( assetIdTo ) )
    {
        // No, so warn.
        Con::warnf("Asset Manager: Cannot rename referenced asset Id '%s' to asset Id '%s' as target asset Id is not declared.", assetIdFrom, assetIdTo );
        return false;
    }

    // Rename asset references.
    renameAssetReferences( assetIdFrom, assetIdTo );

    // Info.
    if ( mEchoInfo )
        Con::printSeparator();

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::releaseAsset( const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_ReleaseAsset);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot release nullptr asset Id." );
   assert(pAssetId != nullptr);

    // Find asset.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Failed to release asset Id '%s' as it does not exist.", pAssetId );
        return false;
    }

    // Is the asset loaded?
    if ( pAssetDefinition->mpAssetBase == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Failed to release asset Id '%s' as it is not acquired.", pAssetId );
        return false;
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Started releasing Asset Id '%s'...", pAssetId );
    }

    // Release asset reference.
    if ( pAssetDefinition->mpAssetBase->releaseAssetReference() )
    {
        // Are we ignoring auto-unloaded assets?
        if ( mIgnoreAutoUnload )
        {
            // Yes, so info.
            if ( mEchoInfo )
            {
                Con::printf( "Asset Manager: > Releasing to idle state." );
            }
        }
        else
        {
            // No, so info.
            if ( mEchoInfo )
            {
                Con::printf( "Asset Manager: > Unload the asset from memory." );
            }

            // Unload the asset.
            unloadAsset( pAssetDefinition );
        }
    }
    // Info.
    else if ( mEchoInfo )
    {
        Con::printf( "Asset Manager: > Reference count now '%d'.", pAssetDefinition->mpAssetBase->getAcquiredReferenceCount() );
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Asset Manager: > Finished releasing Asset Id '%s'.", pAssetId );
        Con::printSeparator();
    }

    return true;
}

//-----------------------------------------------------------------------------

void AssetManager::purgeAssets( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_PurgeAssets);

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Asset Manager: Started purging assets..." );
    }

    // Iterate asset definitions.
    for( auto assetItr:mDeclaredAssets )
    {
        // Fetch asset definition.
        AssetDefinition* pAssetDefinition = assetItr.second;

        // Skip asset if private, not loaded or referenced.
        if (    pAssetDefinition->mAssetPrivate ||
                pAssetDefinition->mpAssetBase == nullptr ||
                pAssetDefinition->mpAssetBase->getAcquiredReferenceCount() > 0 )
            continue;

        // Info.
        if ( mEchoInfo )
        {
            Con::printf( "Asset Manager: Purging asset Id '%s'...", pAssetDefinition->mAssetId );
        }

        // Unload the asset.
        unloadAsset( pAssetDefinition );
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Asset Manager: ... Finished purging assets." );
    }
}

//-----------------------------------------------------------------------------

bool AssetManager::deleteAsset( const char* pAssetId, const bool deleteLooseFiles, const bool deleteDependencies )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_DeleteAsset);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot delete nullptr asset Id." );
   assert(pAssetId != nullptr);

    // Find asset.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Failed to delete asset Id '%s' as it does not exist.", pAssetId );
        return false;
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Started deleting Asset Id '%s'...", pAssetId );
    }  

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Are we deleting dependencies?
    if ( deleteDependencies )
    {
        Vector<typeAssetId> dependantAssets;

        // Yes, so find depended-on-by entry.
        typeAssetDependsOnHash::iterator dependedOnItr = mAssetIsDependedOn.find( assetId );

        // Iterate all dependencies.
        while( dependedOnItr != mAssetIsDependedOn.end() && dependedOnItr->first == assetId )
        {
            // Store asset Id.
            dependantAssets.push_back( dependedOnItr->second );

            // Next dependency.
            dependedOnItr++;
        }

        // Do we have any dependants?
        if ( dependantAssets.size() > 0 )
        {
            // Yes, so iterate dependants.
            for( StringTableEntry dependentAssetId:dependantAssets )
            {
                // Info.
                if ( mEchoInfo )
                {
                    Con::printSeparator();
                    Con::printf( "Asset Manager: Deleting Asset Id '%s' dependant of '%s.'", pAssetId, dependentAssetId );
                }

                // Delete dependant.
                deleteAsset( dependentAssetId, deleteLooseFiles, deleteDependencies );
            }
        }
    }

    // Remove asset references.
    removeAssetReferences( assetId );

    // Are we deleting loose files?
    if ( deleteLooseFiles )
    {
        // Yes, so remove loose files.
        for( StringTableEntry looseFile:pAssetDefinition->mAssetLooseFiles )
        {
            // Delete the loose file.
            if ( !Platform::fileDelete( looseFile ) )
            {
                // Failed so warn.
                Con::warnf( "Asset Manager: Failed to delete the loose file '%s' while deleting asset Id '%s'.", looseFile, pAssetId );
            }
        }
    }

    // Fetch asset definition file.
    StringTableEntry assetDefinitionFile = pAssetDefinition->mAssetBaseFilePath;

    // Remove reference here as we're about to remove the declared asset.
    pAssetDefinition = nullptr;

    // Remove asset.
    removeDeclaredAsset( pAssetId );

    // Delete the asset definition file.
    if ( !Platform::fileDelete( assetDefinitionFile ) )
    {
        // Failed so warn.
        Con::warnf( "Asset Manager: Failed to delete the asset definition file '%s' while deleting asset Id '%s'.", assetDefinitionFile, pAssetId );
    }       

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Finished deleting Asset Id '%s'.", pAssetId );
    }

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::refreshAsset( const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RefreshAsset);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot refresh nullptr asset Id." );
   assert(pAssetId != nullptr);

    // Find asset.
    AssetDefinition* pAssetDefinition = findAsset( pAssetId );

    // Did we find the asset?
    if ( pAssetDefinition == nullptr )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Failed to refresh asset Id '%s' as it does not exist.", pAssetId );
        return false;
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Started refreshing Asset Id '%s'...", pAssetId );
    }    

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Is the asset private?
    if ( pAssetDefinition->mAssetPrivate )
    {
        // Yes, so notify asset of asset refresh only.
        pAssetDefinition->mpAssetBase->onAssetRefresh();

        // Asset refresh notifications.
        for( auto refreshNotifyItr:mAssetPtrRefreshNotifications )
        {
            // Fetch pointed asset.
            StringTableEntry pointedAsset = refreshNotifyItr.first->getAssetId();

            // Ignore if the pointed asset is not the asset or a dependency.
            if ( pointedAsset == StringTable->EmptyString || ( pointedAsset != assetId && !doesAssetDependOn( pointedAsset, assetId ) ) )
                continue;

            // Perform refresh notification callback.
            refreshNotifyItr.second->onAssetRefreshed( refreshNotifyItr.first );
        }
    }
    // Is the asset definition allowed to refresh?
    else if ( pAssetDefinition->mAssetRefreshEnable )
    {
        // Yes, so fetch the asset.
        AssetBase* pAssetBase = pAssetDefinition->mpAssetBase;

        // Is the asset loaded?
        if ( pAssetBase != nullptr )
        {
            // Yes, so notify asset of asset refresh.
            pAssetBase->onAssetRefresh();

            // Save asset.
            mTaml.write( pAssetBase, pAssetDefinition->mAssetBaseFilePath );
        
            // Remove asset dependencies.
            removeAssetDependencies( pAssetId );

            // Find any new dependencies.
            TamlAssetDeclaredVisitor assetDeclaredVisitor;

            // Parse the filename.
            if ( !mTaml.parse( pAssetDefinition->mAssetBaseFilePath, assetDeclaredVisitor ) )
            {
                // Warn.
                Con::warnf( "Asset Manager: Failed to parse file containing asset declaration: '%s'.\nDependencies are now incorrect!", pAssetDefinition->mAssetBaseFilePath );
                return false;
            }

            // Fetch asset dependencies.
            TamlAssetDeclaredVisitor::typeAssetIdVector& assetDependencies = assetDeclaredVisitor.getAssetDependencies();

            // Are there any asset dependences?
            if ( assetDependencies.size() > 0 )
            {
                // Yes, so iterate dependencies.
                for( StringTableEntry dependencyAssetId:assetDependencies )
                {
                    // Insert depends-on.
                    mAssetDependsOn.insertEqual( assetId, dependencyAssetId );

                    // Insert is-depended-on.
                    mAssetIsDependedOn.insertEqual( dependencyAssetId, assetId );
                }
            }

            // Fetch asset loose files.
            TamlAssetDeclaredVisitor::typeLooseFileVector& assetLooseFiles = assetDeclaredVisitor.getAssetLooseFiles();

            // Clear any existing loose files.
            pAssetDefinition->mAssetLooseFiles.clear();

            // Are there any loose files?
            if ( assetLooseFiles.size() > 0 )
            {
                // Yes, so iterate loose files.
                for( auto assetLooseFileItr:assetLooseFiles )
                {
                    // Store loose file.
                    pAssetDefinition->mAssetLooseFiles.push_back( assetLooseFileItr );
                }
            }

            // Asset refresh notifications.
            for( auto refreshNotifyItr:mAssetPtrRefreshNotifications )
            {
                // Fetch pointed asset.
                StringTableEntry pointedAsset = refreshNotifyItr.first->getAssetId();

                // Ignore if the pointed asset is not the asset or a dependency.
                if ( pointedAsset == StringTable->EmptyString || ( pointedAsset != assetId && !doesAssetDependOn( pointedAsset, assetId ) ) )
                    continue;

                // Perform refresh notification callback.
                refreshNotifyItr.second->onAssetRefreshed( refreshNotifyItr.first );
            }

            // Find is-depends-on entry.
            typeAssetIsDependedOnHash::iterator isDependedOnItr = mAssetIsDependedOn.find( assetId );

            // Is asset depended on?
            if ( isDependedOnItr != mAssetIsDependedOn.end() )
            {
                // Yes, so compiled them.
                Vector<typeAssetId> dependedOn;

                // Iterate all dependencies.
                while( isDependedOnItr != mAssetIsDependedOn.end() && isDependedOnItr->first == assetId )
                {
                    dependedOn.push_back( isDependedOnItr->second );

                    // Next dependency.
                    isDependedOnItr++;
                }

                // Refresh depended-on assets.
                for ( typeAssetId isDependedOnItr:dependedOn )
                {
                    // Refresh dependency asset.
                    refreshAsset( isDependedOnItr );
                }
            }
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Finished refreshing Asset Id '%s'.", pAssetId );
    }

    return true;
}

//-----------------------------------------------------------------------------

void AssetManager::refreshAllAssets( const bool includeUnloaded )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RefreshAllAssets);

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Started refreshing ALL assets." );
    }

    Vector<typeAssetId> assetsToRelease;

    // Are we including unloaded assets?
    if ( includeUnloaded )
    {
        // Yes, so prepare a list of assets to release and load them.
        for( auto assetItr:mDeclaredAssets )
        {
            // Fetch asset Id.
            typeAssetId assetId = assetItr.first;

            // Skip if asset is loaded.
            if ( assetItr.second->mpAssetBase != nullptr )
                continue;

            // Note asset as needing a release.
            assetsToRelease.push_back( assetId );

            // Acquire the asset.
            acquireAsset<AssetBase>( assetId );
        }
    }

    // Refresh the current loaded assets.
    // NOTE: This will result in some assets being refreshed more than once due to asset dependencies.
    for( auto assetItr:mDeclaredAssets )
    {
        // Skip private assets.
        if ( assetItr.second->mAssetPrivate )
            continue;

        // Refresh asset if it's loaded.
        refreshAsset( assetItr.first );
    }

    // Are we including unloaded assets?
    if ( includeUnloaded )
    {
        // Yes, so release the assets we loaded.
        for( typeAssetId assetItr:assetsToRelease )
        {
            releaseAsset( assetItr );
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Finished refreshing ALL assets." );
    }
}

//-----------------------------------------------------------------------------

void AssetManager::registerAssetPtrRefreshNotify( AssetPtrBase* pAssetPtrBase, AssetPtrCallback* pCallback )
{
    // Find an existing notification iterator.
    typeAssetPtrRefreshHash::iterator notificationItr = mAssetPtrRefreshNotifications.find( pAssetPtrBase );

    // Do we have one?
    if ( notificationItr != mAssetPtrRefreshNotifications.end() )
    {
        // Yes, so update the callback.
        notificationItr->second = pCallback;
        return;
    }

    // No, so add one.
    mAssetPtrRefreshNotifications.insert( pAssetPtrBase, pCallback );
}

//-----------------------------------------------------------------------------

void AssetManager::unregisterAssetPtrRefreshNotify( AssetPtrBase* pAssetPtrBase )
{
    mAssetPtrRefreshNotifications.erase( pAssetPtrBase );
}

//-----------------------------------------------------------------------------

bool AssetManager::loadAssetTags( ModuleDefinition* pModuleDefinition )
{
    // Sanity!
    AssertFatal( pModuleDefinition != nullptr, "Cannot load asset tags manifest using a nullptr module definition" );
   assert(pModuleDefinition != nullptr);

    // Expand manifest location.
    char assetTagsManifestFilePathBuffer[1024];
    Con::expandPath( assetTagsManifestFilePathBuffer, sizeof(assetTagsManifestFilePathBuffer), pModuleDefinition->getAssetTagsManifest() );

    // Do we already have a manifest?
    if ( !mAssetTagsManifest.isNull() )
    {
        // Yes, so warn.
        Con::warnf( "Asset Manager: Cannot load asset tags manifest from module '%s' as one is already loaded.", pModuleDefinition->getSignature() );
        return false;
    }

    // Is the specified file valid?
    if ( Platform::isFile( assetTagsManifestFilePathBuffer ) )
    {
        // Yes, so read asset tags manifest.
        mAssetTagsManifest = mTaml.read<AssetTagsManifest>( assetTagsManifestFilePathBuffer );

        // Did we read the manifest?
        if ( mAssetTagsManifest.isNull() )
        {
            // No, so warn.
            Con::warnf( "Asset Manager: Failed to load asset tags manifest '%s' from module '%s'.", assetTagsManifestFilePathBuffer, pModuleDefinition->getSignature() );
            return false;
        }

        // Set asset tags module definition.
        mAssetTagsModuleDefinition = pModuleDefinition;
    }
    else
    {
        // No, so generate a new asset tags manifest.
        mAssetTagsManifest = new AssetTagsManifest();
        mAssetTagsManifest->registerObject();

        // Set asset tags module definition.
        mAssetTagsModuleDefinition = pModuleDefinition;

        // Save the asset tags.
        saveAssetTags();
    }

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::saveAssetTags( void )
{
    // Do we have an asset tags manifest?
    if ( mAssetTagsManifest.isNull() || mAssetTagsModuleDefinition.isNull() )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Failed to save asset tags manifest as one is not loaded." );
        return false;
    }

    // Expand manifest location.
    char assetTagsManifestFilePathBuffer[1024];
    Con::expandPath( assetTagsManifestFilePathBuffer, sizeof(assetTagsManifestFilePathBuffer), mAssetTagsModuleDefinition->getAssetTagsManifest() );

    // Save asset tags manifest.
    if ( !mTaml.write( mAssetTagsManifest, assetTagsManifestFilePathBuffer ) )
    {
        // Failed so warn.
        Con::warnf( "Asset Manager: Failed to save asset tags manifest '%s' from module '%s'.", assetTagsManifestFilePathBuffer, mAssetTagsModuleDefinition->getSignature() );
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::restoreAssetTags( void )
{
    // Do we already have a manifest?
    if ( mAssetTagsManifest.isNull() )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot restore asset tags manifest as one is not already loaded." );
        return false;
    }

    // Sanity!
    AssertFatal( mAssetTagsModuleDefinition != nullptr, "Cannot restore asset tags manifest as module definition is nullptr." );

    // Delete existing asset tags manifest.
    mAssetTagsManifest->deleteObject();

    // Reload asset tags manifest.
    return loadAssetTags( mAssetTagsModuleDefinition );
}

//-----------------------------------------------------------------------------

bool descendingAssetDefinitionLoadCount(const AssetDefinition* a, const AssetDefinition* b)
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_DescendingAssetDefinitionLoadCount);

    // Sort.
    return a->mAssetLoadedCount < b->mAssetLoadedCount;
}

//-----------------------------------------------------------------------------

void AssetManager::dumpDeclaredAssets( void ) const
{
    Vector<const AssetDefinition*> assetDefinitions;

    // Iterate asset definitions.
    for( auto assetItr:mDeclaredAssets )
    {
        assetDefinitions.push_back( assetItr.second );
    }

    // Sort asset definitions.
    std::sort( assetDefinitions.begin(), assetDefinitions.end(), descendingAssetDefinitionLoadCount );

    // Info.
    Con::printSeparator();
    Con::printf( "Asset Manager: %d declared asset(s) dump as follows:", mDeclaredAssets.size() );
    Con::printBlankLine();

    // Iterate sorted asset definitions.
    for ( const AssetDefinition* pAssetDefinition: assetDefinitions )
    {
        // Info.
        Con::printf( "AssetId:'%s', RefCount:%d, LoadCount:%d, UnloadCount:%d, AutoUnload:%d, Loaded:%d, Internal:%d, Private: %d, Type:'%s', Module/Version:'%s'/'%d', File:'%s'",
            pAssetDefinition->mAssetId,
            pAssetDefinition->mpAssetBase == nullptr ? 0 : pAssetDefinition->mpAssetBase->getAcquiredReferenceCount(),
            pAssetDefinition->mAssetLoadedCount,
            pAssetDefinition->mAssetUnloadedCount,
            pAssetDefinition->mAssetAutoUnload,
            pAssetDefinition->mpAssetBase != nullptr,
            pAssetDefinition->mAssetInternal,
            pAssetDefinition->mAssetPrivate,
            pAssetDefinition->mAssetType,
            pAssetDefinition->mpModuleDefinition->getModuleId(),
            pAssetDefinition->mpModuleDefinition->getVersionId(),
            pAssetDefinition->mAssetBaseFilePath );
    }

    // Info.
    Con::printSeparator();
    Con::printBlankLine();
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAllAssets( AssetQuery* pAssetQuery, const bool ignoreInternal, const bool ignorePrivate )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAllAssets);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
   assert(pAssetQuery != nullptr);

    // Reset result count.
    S32 resultCount = 0;

    // Iterate declared assets.
    for( auto assetItr : mDeclaredAssets )
    {
        // Fetch asset definition.
        AssetDefinition* pAssetDefinition = assetItr.second;

        // Skip if internal and we're ignoring them.
        if ( ignoreInternal && pAssetDefinition->mAssetInternal )
            continue;

        // Skip if private and we're ignoring them.
        if ( ignorePrivate && pAssetDefinition->mAssetPrivate )
            continue;

        // Store as result.
        pAssetQuery->push_back( pAssetDefinition->mAssetId );

        // Increase result count.
        resultCount++;
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAssetName( AssetQuery* pAssetQuery, const char* pAssetName, const bool partialName )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetName);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
    AssertFatal( pAssetName != nullptr, "Cannot use nullptr asset name." );
   assert(pAssetQuery != nullptr);
   assert(pAssetName != nullptr);

    // Reset asset name.
    StringTableEntry assetName = nullptr;
    dsize_t partialAssetNameLength = 0;
        
    // Are we doing partial name search?
    if ( partialName ) 
    {
        // Yes, so fetch length of partial name.
        partialAssetNameLength = dStrlen( pAssetName );
    }
    else
    {
        // No, so fetch asset name.
        assetName = StringTable->insert( pAssetName );
    }

    // Reset result count.
    S32 resultCount = 0;

    // Iterate declared assets.
    for( auto assetItr : mDeclaredAssets )
    {
        // Fetch asset definition.
        AssetDefinition* pAssetDefinition = assetItr.second;

        // Are we doing partial name search?
        if ( partialName ) 
        {
            // Yes, so fetch the length of this asset name.
            const dsize_t currentAssetNameLength = dStrlen( pAssetDefinition->mAssetName );

            // Skip if the query asset name is longer than the current asset name.
            if ( partialAssetNameLength > currentAssetNameLength )
                continue;
            
            // Skip if this is not the asset we want.
            if ( dStrnicmp( pAssetDefinition->mAssetName, pAssetName, partialAssetNameLength ) != 0 )
                continue;
        }
        else
        {
            // No, so skip if this is not the asset we want.
            if ( assetName != pAssetDefinition->mAssetName )
                continue;
        }

        // Store as result.
        pAssetQuery->push_back( pAssetDefinition->mAssetId );

        // Increase result count.
        resultCount++;
    }

    return resultCount;
}
    
//-----------------------------------------------------------------------------

S32 AssetManager::findAssetCategory( AssetQuery* pAssetQuery, const char* pAssetCategory, const bool assetQueryAsSource )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetCategory);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
    AssertFatal( pAssetCategory != nullptr, "Cannot use nullptr asset category." );
   assert(pAssetQuery != nullptr);
   assert(pAssetCategory != nullptr);

    // Fetch asset category.
    StringTableEntry assetCategory = StringTable->insert( pAssetCategory );

    // Reset result count.
    S32 resultCount = 0;

    // Use asset-query as the source?
    if ( assetQueryAsSource )
    {
        AssetQuery filteredAssets;

        // Yes, so iterate asset query.
        for( auto assetItr : *pAssetQuery )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = findAsset( assetItr );

            // Skip if this is not the asset we want.
            if (    pAssetDefinition == nullptr ||
                    pAssetDefinition->mAssetCategory != assetCategory )
                        continue;

            // Store as result.
            filteredAssets.push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }

        // Set asset query.
        pAssetQuery->set( filteredAssets );
    }
    else
    {
        // No, so iterate declared assets.
        for( auto assetItr : mDeclaredAssets )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = assetItr.second;

            // Skip if this is not the asset we want.
            if ( assetCategory != pAssetDefinition->mAssetCategory )
                continue;

            // Store as result.
            pAssetQuery->push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }
    }

    return resultCount;
}

S32 AssetManager::findAssetAutoUnload( AssetQuery* pAssetQuery, const bool assetAutoUnload, const bool assetQueryAsSource )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetAutoUnload);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
   assert(pAssetQuery != nullptr);

    // Reset result count.
    S32 resultCount = 0;

    // Use asset-query as the source?
    if ( assetQueryAsSource )
    {
        AssetQuery filteredAssets;

        // Yes, so iterate asset query.
        for( auto assetItr : *pAssetQuery )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = findAsset( assetItr );

            // Skip if this is not the asset we want.
            if (    pAssetDefinition == nullptr ||
                    pAssetDefinition->mAssetAutoUnload != assetAutoUnload )
                        continue;

            // Store as result.
            filteredAssets.push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }

        // Set asset query.
        pAssetQuery->set( filteredAssets );
    }
    else
    {
        // No, so iterate declared assets.
        for( auto assetItr : mDeclaredAssets )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = assetItr.second;

            // Skip if this is not the asset we want.
            if ( assetAutoUnload != pAssetDefinition->mAssetAutoUnload )
                continue;

            // Store as result.
            pAssetQuery->push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAssetInternal( AssetQuery* pAssetQuery, const bool assetInternal, const bool assetQueryAsSource )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetInternal);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
   assert(pAssetQuery != nullptr);

    // Reset result count.
    S32 resultCount = 0;

    // Use asset-query as the source?
    if ( assetQueryAsSource )
    {
        AssetQuery filteredAssets;

        // Yes, so iterate asset query.
        for( auto assetItr : *pAssetQuery )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = findAsset( assetItr );

            // Skip if this is not the asset we want.
            if (    pAssetDefinition == nullptr ||
                    pAssetDefinition->mAssetInternal != assetInternal )
                        continue;

            // Store as result.
            filteredAssets.push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }

        // Set asset query.
        pAssetQuery->set( filteredAssets );
    }
    else
    {
        // No, so iterate declared assets.
        for( auto assetItr : mDeclaredAssets )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = assetItr.second;

            // Skip if this is not the asset we want.
            if ( assetInternal != pAssetDefinition->mAssetInternal )
                continue;

            // Store as result.
            pAssetQuery->push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAssetPrivate( AssetQuery* pAssetQuery, const bool assetPrivate, const bool assetQueryAsSource )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetPrivate);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
   assert(pAssetQuery != nullptr);

    // Reset result count.
    S32 resultCount = 0;

    // Use asset-query as the source?
    if ( assetQueryAsSource )
    {
        AssetQuery filteredAssets;

        // Yes, so iterate asset query.
        for( auto assetItr : *pAssetQuery )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = findAsset( assetItr );

            // Skip if this is not the asset we want.
            if (    pAssetDefinition == nullptr ||
                    pAssetDefinition->mAssetPrivate != assetPrivate )
                        continue;

            // Store as result.
            filteredAssets.push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }

        // Set asset query.
        pAssetQuery->set( filteredAssets );
    }
    else
    {
        // No, so iterate declared assets.
        for( auto assetItr : mDeclaredAssets )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = assetItr.second;

            // Skip if this is not the asset we want.
            if ( assetPrivate != pAssetDefinition->mAssetPrivate )
                continue;

            // Store as result.
            pAssetQuery->push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAssetType( AssetQuery* pAssetQuery, const char* pAssetType, const bool assetQueryAsSource )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetType);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
    AssertFatal( pAssetType != nullptr, "Cannot use nullptr asset type." );
   assert(pAssetQuery != nullptr);
   assert(pAssetType != nullptr);

    // Fetch asset type.
    StringTableEntry assetType = StringTable->insert( pAssetType );

    // Reset result count.
    S32 resultCount = 0;

    // Use asset-query as the source?
    if ( assetQueryAsSource )
    {
        AssetQuery filteredAssets;

        // Yes, so iterate asset query.
        for( auto assetItr : *pAssetQuery )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = findAsset( assetItr );

            // Skip if this is not the asset we want.
            if (    pAssetDefinition == nullptr ||
                    pAssetDefinition->mAssetType != assetType )
                        continue;

            // Store as result.
            filteredAssets.push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }

        // Set asset query.
        pAssetQuery->set( filteredAssets );
    }
    else
    {
        // No, so iterate declared assets.
        for( auto assetItr: mDeclaredAssets )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = assetItr.second;

            // Skip if this is not the asset we want.
            if ( assetType != pAssetDefinition->mAssetType )
                continue;

            // Store as result.
            pAssetQuery->push_back( pAssetDefinition->mAssetId );

            // Increase result count.
            resultCount++;
        }
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAssetDependsOn( AssetQuery* pAssetQuery, const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetDependsOn);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
    AssertFatal( pAssetId != nullptr, "Cannot use nullptr asset Id." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Reset result count.
    S32 resultCount = 0;

    // Find depends-on entry.
    typeAssetDependsOnHash::iterator dependsOnItr = mAssetDependsOn.find( assetId );

    // Iterate all dependencies.
    while( dependsOnItr != mAssetDependsOn.end() && dependsOnItr->first == assetId )
    {
        // Store as result.
        pAssetQuery->push_back( dependsOnItr->second );

        // Next dependency.
        dependsOnItr++;

        // Increase result count.
        resultCount++;
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAssetIsDependedOn( AssetQuery* pAssetQuery, const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetIsDependedOn);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
    AssertFatal( pAssetId != nullptr, "Cannot use nullptr asset Id." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Reset result count.
    S32 resultCount = 0;

    // Find depended-on entry.
    typeAssetIsDependedOnHash::iterator dependedOnItr = mAssetIsDependedOn.find( assetId );

    // Iterate all dependencies.
    while( dependedOnItr != mAssetIsDependedOn.end() && dependedOnItr->first == assetId )
    {
        // Store as result.
        pAssetQuery->push_back( dependedOnItr->second );

        // Next dependency.
        dependedOnItr++;

        // Increase result count.
        resultCount++;
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findInvalidAssetReferences( AssetQuery* pAssetQuery )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindInvalidAssetReferences);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );

    // Reset result count.
    S32 resultCount = 0;

    // Iterate referenced assets.
    for( auto assetItr : mReferencedAssets )
    {
        // Find asset definition.
        AssetDefinition* pAssetDefinition = findAsset( assetItr.first );

        // Skip if the asset definition was found.
        if ( pAssetDefinition != nullptr )
            continue;

        // Store as result.
        pAssetQuery->push_back( assetItr.first );

        // Increase result count.
        resultCount++;
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findTaggedAssets( AssetQuery* pAssetQuery, const char* pAssetTagNames, const bool assetQueryAsSource )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindTaggedAssets);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
    AssertFatal( pAssetTagNames != nullptr, "Cannot use nullptr asset tag name(s)." );

    // Do we have an asset tag manifest?
    if ( mAssetTagsManifest.isNull() )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Cannot find tagged assets as no asset tag manifest is present." );
        return 0;
    }

    // Reset result count.
    S32 resultCount = 0;

    const char* pTagSeparators = " ,\t\n";

    // Fetch tag count.
    dsize_t assetTagCount = StringUnit::getUnitCount( pAssetTagNames, pTagSeparators );

    // Fetch asset tags.
    Vector<AssetTagsManifest::AssetTag*> assetTags;
    for( U32 tagIndex = 0; tagIndex < assetTagCount; ++tagIndex )
    {
        // Fetch asset tag name.
        const char* pTagName = StringUnit::getUnit( pAssetTagNames, tagIndex, pTagSeparators );

        // Fetch asset tag.
        AssetTagsManifest::AssetTag* pAssetTag = mAssetTagsManifest->findAssetTag( pTagName );

        // Did we find the asset tag?
        if ( pAssetTag == nullptr )
        {
            // No, so warn.
            Con::warnf( "AssetTagsManifest: Asset Manager: Cannot find tagged assets of '%s' as it does not exist.  Ignoring tag.", pTagName );
            continue;
        }

        assetTags.push_back( pAssetTag );
    }

    // Fetch found asset tag count.
    assetTagCount = assetTags.size();

    // Did we find any tags?
    if ( assetTagCount == 0 )
    {
        // No, so warn.
        Con::warnf( "AssetTagsManifest: Asset Manager: No specified tagged assets found in '%s'.", pAssetTagNames );
        return 0;
    } 

    // Use asset-query as the source?
    if ( assetQueryAsSource )
    {
        AssetQuery filteredAssets;

        // Yes, so iterate asset query.
        for( auto assetId : *pAssetQuery )
        {
            // Skip if asset is not valid.
            if ( !isDeclaredAsset( assetId ) )
                continue;

            // Reset matched flag.
            bool assetTagMatched = false;

            // Iterate asset tags.
            for ( AssetTagsManifest::AssetTag* pAssetTag : assetTags )
            {
                // Skip if asset is not tagged.
                if ( !pAssetTag->containsAsset( assetId ) )
                    continue;
                
                // Flag as matched.
                assetTagMatched = true;
                break;
            }

            // Did we find a match?
            if ( assetTagMatched )
            {
                // Yes, so is asset already present?
                if ( !filteredAssets.containsAsset( assetId ) )
                {
                    // No, so store as result.
                    filteredAssets.push_back( assetId );

                    // Increase result count.
                    resultCount++;
                }
            }
        }

        // Set asset query.
        pAssetQuery->set( filteredAssets );
    }
    else
    {
        // Iterate asset tags.
        for ( AssetTagsManifest::AssetTag* pAssetTag : assetTags  )
        {
            // Iterate tagged assets.
            for ( StringTableEntry assetId :pAssetTag->mAssets )
            {
                // Skip if asset Id is already present.
                if ( pAssetQuery->containsAsset( assetId ) )
                    continue;

                // Store as result.
                pAssetQuery->push_back( assetId );

                // Increase result count.
                resultCount++;
            }
        }
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

S32 AssetManager::findAssetLooseFile( AssetQuery* pAssetQuery, const char* pLooseFile, const bool assetQueryAsSource )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAssetLooseFile);

    // Sanity!
    AssertFatal( pAssetQuery != nullptr, "Cannot use nullptr asset query." );
    AssertFatal( pLooseFile != nullptr, "Cannot use nullptr loose file." );

    // Expand loose file.
    char looseFileBuffer[1024];
    Con::expandPath(looseFileBuffer, sizeof(looseFileBuffer), pLooseFile, false );

    // Fetch asset loose file.
    StringTableEntry looseFile = StringTable->insert( looseFileBuffer );

    // Reset result count.
    S32 resultCount = 0;

    // Use asset-query as the source?
    if ( assetQueryAsSource )
    {
        AssetQuery filteredAssets;

        // Yes, so iterate asset query.
        for( auto assetItr : *pAssetQuery )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = findAsset( assetItr );

            // Fetch loose files.
            Vector<StringTableEntry>& assetLooseFiles = pAssetDefinition->mAssetLooseFiles;

            // Skip if this asset has no loose files.
            if ( assetLooseFiles.size() == 0 )
                continue;

            // Search the assets loose files.
            for( auto looseFileItr : assetLooseFiles )
            {
                // Is this the loose file we are searching for?
                if ( looseFileItr != looseFile )
                    continue;

                // Store as result.
                filteredAssets.push_back( pAssetDefinition->mAssetId );

                // Increase result count.
                resultCount++;

                break;
            }
        }

        // Set asset query.
        pAssetQuery->set( filteredAssets );
    }
    else
    {
        // No, so iterate declared assets.
        for( auto assetItr:mDeclaredAssets )
        {
            // Fetch asset definition.
            AssetDefinition* pAssetDefinition = assetItr.second;

            // Fetch loose files.
            Vector<StringTableEntry>& assetLooseFiles = pAssetDefinition->mAssetLooseFiles;

            // Skip if this asset has no loose files.
            if ( assetLooseFiles.size() == 0 )
                continue;

            // Search the assets loose files.
            for( auto looseFileItr : assetLooseFiles )
            {
                // Is this the loose file we are searching for?
                if ( looseFileItr != looseFile )
                    continue;

                // Store as result.
                pAssetQuery->push_back( pAssetDefinition->mAssetId );

                // Increase result count.
                resultCount++;

                break;
            }
        }
    }

    return resultCount;
}

//-----------------------------------------------------------------------------

bool AssetManager::scanDeclaredAssets( const char* pPath, const char* pExtension, const bool recurse, ModuleDefinition* pModuleDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_ScanDeclaredAssets);

    // Sanity!
    AssertFatal( pPath != nullptr, "Cannot scan declared assets with nullptr path." );
    AssertFatal( pExtension != nullptr, "Cannot scan declared assets with nullptr extension." );

    // Expand path location.
    char pathBuffer[1024];
    Con::expandPath( pathBuffer, sizeof(pathBuffer), pPath );

    // Find files.
    Vector<Platform::FileInfo> files;
    if ( !Platform::dumpPath( pathBuffer, files, recurse ? -1 : 0 ) )
    {
        // Failed so warn.
        Con::warnf( "Asset Manager: Failed to scan declared assets in directory '%s'.", pathBuffer );
        return false;
    }

    // Is the asset file-path located within the specified module?
    if ( !Con::isBasePath( pathBuffer, pModuleDefinition->getModulePath() ) )
    {
        // No, so warn.
        Con::warnf( "Asset Manager: Could not add declared asset file '%s' as file does not exist with module path '%s'",
            pathBuffer,
            pModuleDefinition->getModulePath() );
        return false;
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Scanning for declared assets in path '%s' for files with extension '%s'...", pathBuffer, pExtension );
    }

    // Fetch extension length.
    const size_t extensionLength = dStrlen( pExtension );

    // Fetch module assets.
    ModuleDefinition::typeModuleAssetsVector& moduleAssets = pModuleDefinition->getModuleAssets();

    TamlAssetDeclaredVisitor assetDeclaredVisitor;

    // Iterate files.
    for ( Platform::FileInfo fileInfo: files )
    {
        // Fetch filename.
        const char* pFilename = fileInfo.pFileName;

        // Find filename length.
        const size_t filenameLength = dStrlen( pFilename );

        // Skip if extension is longer than filename.
        if ( extensionLength > filenameLength )
            continue;

        // Skip if extension not found.
        if ( dStricmp( pFilename + filenameLength - extensionLength, pExtension ) != 0 )
            continue;

        // Clear declared assets.
        assetDeclaredVisitor.clear();

        // Format full file-path.
        char assetFileBuffer[1024];
        dSprintf( assetFileBuffer, sizeof(assetFileBuffer), "%s/%s", fileInfo.pFullPath, fileInfo.pFileName );

        // Parse the filename.
        if ( !mTaml.parse( assetFileBuffer, assetDeclaredVisitor ) )
        {
            // Warn.
            Con::warnf( "Asset Manager: Failed to parse file containing asset declaration: '%s'.", assetFileBuffer );
            continue;
        }

        // Fetch asset definition.
        AssetDefinition& foundAssetDefinition = assetDeclaredVisitor.getAssetDefinition();

        // Did we get an asset name?
        if ( foundAssetDefinition.mAssetName == StringTable->EmptyString )
        {
            // No, so warn.
            Con::warnf( "Asset Manager: Parsed file '%s' but did not encounter an asset.", assetFileBuffer );
            continue;
        }

        // Set module definition.
        foundAssetDefinition.mpModuleDefinition = pModuleDefinition;

        // Format asset Id.
        char assetIdBuffer[1024];
        dSprintf(assetIdBuffer, sizeof(assetIdBuffer), "%s%s%s",
            pModuleDefinition->getModuleId(),
            ASSET_SCOPE_TOKEN,
            foundAssetDefinition.mAssetName );

        // Set asset Id.
        foundAssetDefinition.mAssetId = StringTable->insert( assetIdBuffer );

        // Does this asset already exist?
        if ( mDeclaredAssets.contains( foundAssetDefinition.mAssetId ) )
        {
            // Yes, so warn.
            Con::warnf( "Asset Manager: Encountered asset Id '%s' in asset file '%s' but it conflicts with existing asset Id in asset file '%s'.",
                foundAssetDefinition.mAssetId,
                foundAssetDefinition.mAssetBaseFilePath,
                mDeclaredAssets.find( foundAssetDefinition.mAssetId )->second->mAssetBaseFilePath );

            continue;
        }

        // Create new asset definition.
        AssetDefinition* pAssetDefinition = new AssetDefinition( foundAssetDefinition );

        // Store in declared assets.
        mDeclaredAssets.insert( pAssetDefinition->mAssetId, pAssetDefinition );

        // Store in module assets.
        moduleAssets.push_back( pAssetDefinition );
        
        // Info.
        if ( mEchoInfo )
        {
            Con::printSeparator();
            Con::printf( "Asset Manager: Adding Asset Id '%s' of type '%s' in asset file '%s'.",
                pAssetDefinition->mAssetId,
                pAssetDefinition->mAssetType,
                pAssetDefinition->mAssetBaseFilePath );
        }

        // Fetch asset Id.
        StringTableEntry assetId = pAssetDefinition->mAssetId;

        // Fetch asset dependencies.
        TamlAssetDeclaredVisitor::typeAssetIdVector& assetDependencies = assetDeclaredVisitor.getAssetDependencies();

        // Are there any asset dependencies?
        if ( assetDependencies.size() > 0 )
        {
            // Yes, so iterate dependencies.
            for( StringTableEntry dependencyAssetId  : assetDependencies  )
            {
                // Insert depends-on.
                mAssetDependsOn.insertEqual( assetId, dependencyAssetId );

                // Insert is-depended-on.
                mAssetIsDependedOn.insertEqual( dependencyAssetId, assetId );

                // Info.
                if ( mEchoInfo )
                {
                    Con::printf( "Asset Manager: Asset Id '%s' has dependency of Asset Id '%s'", assetId, dependencyAssetId );
                }
            }
        }

        // Fetch asset loose files.
        TamlAssetDeclaredVisitor::typeLooseFileVector& assetLooseFiles = assetDeclaredVisitor.getAssetLooseFiles();

        // Are there any loose files?
        if ( assetLooseFiles.size() > 0 )
        {
            // Yes, so iterate loose files.
            for( StringTableEntry looseFile: assetDeclaredVisitor.getAssetLooseFiles())
            {
                // Info.
                if ( mEchoInfo )
                {
                    Con::printf( "Asset Manager: Asset Id '%s' has loose file '%s'.", assetId, looseFile );
                }

                // Store loose file.
                pAssetDefinition->mAssetLooseFiles.push_back( looseFile );
            }
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: ... Finished scanning for declared assets in path '%s' for files with extension '%s'.", pathBuffer, pExtension );
        Con::printSeparator();
        Con::printBlankLine();
    }

    return true;
}

//-----------------------------------------------------------------------------

bool AssetManager::scanReferencedAssets( const char* pPath, const char* pExtension, const bool recurse )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_ScanReferencedAssets);

    // Sanity!
    AssertFatal( pPath != nullptr, "Cannot scan referenced assets with nullptr path." );
    AssertFatal( pExtension != nullptr, "Cannot scan referenced assets with nullptr extension." );

    // Expand path location.
    char pathBuffer[1024];
    Con::expandPath( pathBuffer, sizeof(pathBuffer), pPath );

    // Find files.
    Vector<Platform::FileInfo> files;
    if ( !Platform::dumpPath( pathBuffer, files, recurse ? -1 : 0 ) )
    {
        // Failed so warn.
        Con::warnf( "Asset Manager: Failed to scan referenced assets in directory '%s'.", pathBuffer );
        return false;
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Asset Manager: Scanning for referenced assets in path '%s' for files with extension '%s'...", pathBuffer, pExtension );
    }

    // Fetch extension length.
    const dsize_t extensionLength = dStrlen( pExtension );

    TamlAssetReferencedVisitor assetReferencedVisitor;

    // Iterate files.
    for ( Platform::FileInfo fileInfo:files )
    {
        // Fetch filename.
        const char* pFilename = fileInfo.pFileName;

        // Find filename length.
        const dsize_t filenameLength = dStrlen( pFilename );

        // Skip if extension is longer than filename.
        if ( extensionLength > filenameLength )
            continue;

        // Skip if extension not found.
        if ( dStricmp( pFilename + filenameLength - extensionLength, pExtension ) != 0 )
            continue;

        // Clear referenced assets.
        assetReferencedVisitor.clear();

        // Format full file-path.
        char assetFileBuffer[1024];
        dSprintf( assetFileBuffer, sizeof(assetFileBuffer), "%s/%s", fileInfo.pFullPath, fileInfo.pFileName );

        // Format reference file-path.
        typeReferenceFilePath referenceFilePath = StringTable->insert( assetFileBuffer );

        // Parse the filename.
        if ( !mTaml.parse( referenceFilePath, assetReferencedVisitor ) )
        {
            // Warn.
            Con::warnf( "Asset Manager: Failed to parse file containing asset references: '%s'.", referenceFilePath );
            continue;
        }

        // Fetch usage map.
        const TamlAssetReferencedVisitor::typeAssetReferencedHash& assetReferencedMap = assetReferencedVisitor.getAssetReferencedMap();

        // Do we have any asset references?
        if ( assetReferencedMap.size() > 0 )
        {
            // Info.
            if ( mEchoInfo )
            {
                Con::printSeparator();
            }

            // Iterate usage.
            for( auto usageItr : assetReferencedMap  )
            {
                // Fetch asset name.
                typeAssetId assetId = usageItr.first;

                // Info.
                if ( mEchoInfo )
                {
                    Con::printf( "Asset Manager: Found referenced Asset Id '%s' in file '%s'.", assetId, referenceFilePath );
                }

                // Add referenced asset.
                addReferencedAsset( assetId, referenceFilePath );
            }
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Asset Manager: ... Finished scanning for referenced assets in path '%s' for files with extension '%s'.", pathBuffer, pExtension );
        Con::printSeparator();
        Con::printBlankLine();
    }

    return true;
}

//-----------------------------------------------------------------------------

AssetDefinition* AssetManager::findAsset( const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_FindAsset);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot find nullptr asset Id." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Find declared asset.
    typeDeclaredAssetsHash::iterator declaredAssetItr = mDeclaredAssets.find( assetId );

    // Find if we didn't find a declared asset Id.
    if ( declaredAssetItr == mDeclaredAssets.end() )
        return nullptr;

    return declaredAssetItr->second;
}

//-----------------------------------------------------------------------------

void AssetManager::addReferencedAsset( StringTableEntry assetId, StringTableEntry referenceFilePath )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_AddReferencedAsset);

    // Sanity!
    AssertFatal( assetId != nullptr, "Cannot add referenced asset with nullptr asset Id." );
    AssertFatal( referenceFilePath != nullptr, "Cannot add referenced asset with nullptr reference file-path." );

    // Find referenced asset.
    typeReferencedAssetsHash::iterator referencedAssetItr = mReferencedAssets.find( assetId );

    // Did we find the asset?
    if ( referencedAssetItr == mReferencedAssets.end() )
    {
        // No, so add asset Id.
        mReferencedAssets.insertEqual( assetId, referenceFilePath );
    }
    else
    {
        // Yes, so add asset Id with a unique file.
        while( true )
        {
            // Finish if this file is already present.
            if ( referencedAssetItr->second == referenceFilePath )
                return;

            // Move to next asset Id.
            referencedAssetItr++;

            // Is this the end of referenced assets or a different asset Id?
            if ( referencedAssetItr == mReferencedAssets.end() ||
                referencedAssetItr->first != assetId )
            {
                // Yes, so add asset reference.
                mReferencedAssets.insertEqual( assetId, referenceFilePath );
                return;
            }
        };
    }
}

//-----------------------------------------------------------------------------

void AssetManager::renameAssetReferences( StringTableEntry assetIdFrom, StringTableEntry assetIdTo )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RenameAssetReferences);

    // Sanity!
    AssertFatal( assetIdFrom != nullptr, "Cannot rename asset references using nullptr asset Id from." );
    AssertFatal( assetIdTo != nullptr, "Cannot rename asset references using nullptr asset Id to." );

    // Finish if the asset is not referenced.
    if ( !mReferencedAssets.count( assetIdFrom ) )
        return;

    // Setup referenced update visitor.
    TamlAssetReferencedUpdateVisitor assetReferencedUpdateVisitor;
    assetReferencedUpdateVisitor.setAssetIdFrom( assetIdFrom );
    assetReferencedUpdateVisitor.setAssetIdTo( assetIdTo );

    // Find first referenced asset Id.
    typeReferencedAssetsHash::iterator referencedAssetItr = mReferencedAssets.find( assetIdFrom );

    // Iterate references.
    while( true )
    {
        // Finish if end of references.
        if ( referencedAssetItr == mReferencedAssets.end() || referencedAssetItr->first != assetIdFrom )
            return;

        // Info.
        if ( mEchoInfo )
        {
            Con::printf( "Asset Manager: Renaming declared Asset Id '%s' to Asset Id '%s'.  Updating referenced file '%s'",
                assetIdFrom,
                assetIdTo,
                referencedAssetItr->second );
        }

        // Update asset file declaration.
        if ( !mTaml.parse( referencedAssetItr->second, assetReferencedUpdateVisitor ) )
        {
            // No, so warn.
            Con::warnf("Asset Manager: Cannot rename referenced asset Id '%s' to asset Id '%s' as the referenced asset file could not be parsed: %s",
                assetIdFrom, assetIdTo, referencedAssetItr->second );
        }

        // Move to next reference.
        referencedAssetItr++;
    }
}

//-----------------------------------------------------------------------------

void AssetManager::removeAssetReferences( StringTableEntry assetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RemoveAssetReferences);

    // Sanity!
    AssertFatal( assetId != nullptr, "Cannot rename asset references using nullptr asset Id." );

    // Finish if the asset is not referenced.
    if ( !mReferencedAssets.count( assetId ) )
        return;

    // Setup referenced update visitor.
    TamlAssetReferencedUpdateVisitor assetReferencedUpdateVisitor;
    assetReferencedUpdateVisitor.setAssetIdFrom( assetId );
    assetReferencedUpdateVisitor.setAssetIdTo( StringTable->EmptyString );

    // Find first referenced asset Id.
    typeReferencedAssetsHash::iterator referencedAssetItr = mReferencedAssets.find( assetId );

    // Iterate references.
    while( true )
    {
        // Finish if end of references.
        if ( referencedAssetItr == mReferencedAssets.end() || referencedAssetItr->first != assetId )
            break;

        // Info.
        if ( mEchoInfo )
        {
            Con::printf( "Asset Manager: Removing Asset Id '%s' references from file '%s'",
                assetId,
                referencedAssetItr->second );
        }

        // Update asset file declaration.
        if ( !mTaml.parse( referencedAssetItr->second, assetReferencedUpdateVisitor ) )
        {
            // No, so warn.
            Con::warnf("Asset Manager: Cannot remove referenced asset Id '%s' as the referenced asset file could not be parsed: %s",
                assetId,
                referencedAssetItr->second );
        }

        // Move to next reference.
        referencedAssetItr++;
    }

    // Remove asset references.
    mReferencedAssets.erase( assetId );
}

//-----------------------------------------------------------------------------

void AssetManager::renameAssetDependencies( StringTableEntry assetIdFrom, StringTableEntry assetIdTo )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RenameAssetDependencies);

    // Sanity!
    AssertFatal( assetIdFrom != nullptr, "Cannot rename asset dependencies using nullptr asset Id from." );
    AssertFatal( assetIdTo != nullptr, "Cannot rename asset dependencies using nullptr asset Id to." );
   assert(assetIdFrom != nullptr);
   assert(assetIdTo != nullptr);

    // Rename via depends-on...
    while( mAssetDependsOn.count( assetIdFrom ) > 0 )
    {
        // Find depends-on.
        typeAssetDependsOnHash::iterator dependsOnItr = mAssetDependsOn.find( assetIdFrom );

        // Fetch dependency asset Id.
        StringTableEntry dependencyAssetId = dependsOnItr->second;

        // Find is-depends-on entry.
        typeAssetIsDependedOnHash::iterator isDependedOnItr = mAssetIsDependedOn.find( dependencyAssetId );

        // Sanity!
        AssertFatal( isDependedOnItr != mAssetIsDependedOn.end(), "Asset dependencies are corrupt!" );

        while( isDependedOnItr != mAssetIsDependedOn.end() && isDependedOnItr->first == dependencyAssetId && isDependedOnItr->second != assetIdFrom )
        {
            isDependedOnItr++;
        }

        // Sanity!
        AssertFatal( isDependedOnItr->first == dependencyAssetId && isDependedOnItr->second == assetIdFrom, "Asset dependencies are corrupt!" );
        
        // Remove is-depended-on.        
        mAssetIsDependedOn.erase( isDependedOnItr );

        // Remove depends-on.
        mAssetDependsOn.erase( dependsOnItr );

        // Insert depends-on.
        mAssetDependsOn.insertEqual( assetIdTo, dependencyAssetId );

        // Insert is-depended-on.
        mAssetIsDependedOn.insertEqual( dependencyAssetId, assetIdTo );
    }

    // Rename via is-depended-on...
    while( mAssetIsDependedOn.count( assetIdFrom ) > 0 )
    {
        // Find is-depended-on.
        typeAssetIsDependedOnHash::iterator isdependedOnItr = mAssetIsDependedOn.find( assetIdFrom );

        // Fetch dependency asset Id.
        StringTableEntry dependencyAssetId = isdependedOnItr->second;

        // Find depends-on entry.
        typeAssetDependsOnHash::iterator dependsOnItr = mAssetDependsOn.find( dependencyAssetId );

        // Sanity!
        AssertFatal( dependsOnItr != mAssetDependsOn.end(), "Asset dependencies are corrupt!" );

        while( dependsOnItr != mAssetDependsOn.end() && dependsOnItr->first == dependencyAssetId && dependsOnItr->second != assetIdFrom )
        {
            dependsOnItr++;
        }

        // Sanity!
        AssertFatal( dependsOnItr->first == dependencyAssetId && dependsOnItr->second == assetIdFrom, "Asset dependencies are corrupt!" );
        
        // Remove is-depended-on.        
        mAssetIsDependedOn.erase( isdependedOnItr );

        // Remove depends-on.
        mAssetDependsOn.erase( dependsOnItr );

        // Insert depends-on.
        mAssetDependsOn.insertEqual( dependencyAssetId, assetIdTo );

        // Insert is-depended-on.
        mAssetIsDependedOn.insertEqual( assetIdTo, dependencyAssetId );
    }
}

//-----------------------------------------------------------------------------

void AssetManager::removeAssetDependencies( const char* pAssetId )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_RemvoeAsetDependencies);

    // Sanity!
    AssertFatal( pAssetId != nullptr, "Cannot remove asset dependencies using nullptr asset Id." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    auto dependsOnItrSet = mAssetDependsOn.equal_range( assetId );

    for (typeAssetDependsOnHash::iterator dependsOnItr = dependsOnItrSet.first; dependsOnItr != dependsOnItrSet.second; dependsOnItr++)
    {
        // Fetch dependency asset Id.
        StringTableEntry dependencyAssetId = dependsOnItr->second;

        auto isDependedOnItrSet = mAssetIsDependedOn.equal_range( dependencyAssetId );
        typeAssetIsDependedOnHash::iterator isDependedOnItr = isDependedOnItrSet.first;

        while ( isDependedOnItr != isDependedOnItrSet.second)
        {
            if ( isDependedOnItr->second == assetId )
            {
                mAssetIsDependedOn.erase(isDependedOnItr);
                isDependedOnItrSet = mAssetIsDependedOn.equal_range( dependencyAssetId );
                isDependedOnItr = isDependedOnItrSet.first;
            }
            else
            {
               isDependedOnItr++;
            }
        }
    }
   mAssetDependsOn.erase( pAssetId );

}

//-----------------------------------------------------------------------------

void AssetManager::unloadAsset( AssetDefinition* pAssetDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_UnloadAsset);

    // Destroy the asset.
    pAssetDefinition->mpAssetBase->deleteObject();

    // Increase unloaded count.
    pAssetDefinition->mAssetUnloadedCount++;

    // Is the asset internal?
    if ( pAssetDefinition->mAssetInternal )
    {
        // Yes, so decrease internal loaded asset count.
        mLoadedInternalAssetsCount--;
    }
    else
    {
        // No, so decrease external loaded assets count.
        mLoadedExternalAssetsCount--;
    }

    // Is the asset private?
    if ( pAssetDefinition->mAssetPrivate )
    {
        // Yes, so decrease private loaded asset count.
        mLoadedPrivateAssetsCount--;

        // Remove it completely.
        removeDeclaredAsset( pAssetDefinition->mAssetId );
    }
}

//-----------------------------------------------------------------------------

void AssetManager::onModulePreLoad( ModuleDefinition* pModuleDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_OnModulePreLoad);

    // Add module declared assets.
    addModuleDeclaredAssets( pModuleDefinition );

    // Is an asset tags manifest specified?
    if ( pModuleDefinition->getAssetTagsManifest() != StringTable->EmptyString )
    {
        // Yes, so load the asset tags manifest.
        loadAssetTags( pModuleDefinition );
    }
}

//-----------------------------------------------------------------------------

void AssetManager::onModulePreUnload( ModuleDefinition* pModuleDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_OnModulePreUnload);

    // Is an asset tags manifest specified?
    if ( pModuleDefinition->getAssetTagsManifest() != StringTable->EmptyString )
    {
        // Yes, so save the asset tags manifest.
        saveAssetTags();

        // Do we have an asset tags manifest?
        if ( !mAssetTagsManifest.isNull() )
        {
            // Yes, so remove it.
            mAssetTagsManifest->deleteObject();
            mAssetTagsModuleDefinition = nullptr;
        }
    }
}

//-----------------------------------------------------------------------------

void AssetManager::onModulePostUnload( ModuleDefinition* pModuleDefinition )
{
    // Debug Profiling.
    PROFILE_SCOPE(AssetManager_OnModulePostUnload);

    // Remove declared assets.
    removeDeclaredAssets( pModuleDefinition );
}
