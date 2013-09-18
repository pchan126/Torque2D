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

#include "console/consoleTypes.h"
#include "2d/assets/ParticleAsset.h"
#include "ParticleAsset_ScriptBinding.h"
#include "debug/profiler.h"

//------------------------------------------------------------------------------

static EnumTable::Enums LifeModeEntries[4] =
    {
    EnumTable::Enums( ParticleAsset::INFINITE_LIFEMODE,  "INFINITE" ),
    EnumTable::Enums( ParticleAsset::CYCLE,     "CYCLE" ),
    EnumTable::Enums( ParticleAsset::STOP,      "STOP" ),
    EnumTable::Enums( ParticleAsset::KILL,      "KILL" ),
    };

static EnumTable LifeModeTable = EnumTable(4, LifeModeEntries);
//-----------------------------------------------------------------------------

ParticleAsset::LifeMode ParticleAsset::getParticleAssetLifeModeEnum( const char* label )
{
   if (LifeModeTable.isLabel(label))
       return (ParticleAsset::LifeMode)LifeModeTable[label];

   // Warn.
   Con::warnf( "ParticleAsset::getParticleAssetLifeModeEnum() - Invalid life mode '%s'.", label );

   return ParticleAsset::INVALID_LIFEMODE;
}

//-----------------------------------------------------------------------------

const char* ParticleAsset::getParticleAssetLifeModeDescription( const ParticleAsset::LifeMode lifeMode )
{
    if (LifeModeTable.isIndex(lifeMode))
        return LifeModeTable[lifeMode].c_str();

    // Warn.
    Con::warnf( "ParticleAsset::getParticleAssetLifeModeDescription() - Invalid life-mode." );

    return StringTable->EmptyString;
}

//-----------------------------------------------------------------------------

ConsoleType( particleAssetPtr, TypeParticleAssetPtr, sizeof(AssetPtr<ParticleAsset>), ASSET_ID_FIELD_PREFIX )

//-----------------------------------------------------------------------------

ConsoleGetType( TypeParticleAssetPtr )
{
    // Fetch asset Id.
    return (*((AssetPtr<ParticleAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType( TypeParticleAssetPtr )
{
    // Was a single argument specified?
    if( argc == 1 )
    {
        // Yes, so fetch field value.
        const char* pFieldValue = argv[0];

        // Fetch asset pointer.
        AssetPtr<ParticleAsset>* pAssetPtr = dynamic_cast<AssetPtr<ParticleAsset>*>((AssetPtrBase*)(dptr));

        // Is the asset pointer the correct type?
        if ( pAssetPtr == nullptr )
        {
            // No, so fail.
            Con::warnf( "(TypeParticleAssetPtr) - Failed to set asset Id '%d'.", pFieldValue );
            return;
        }

        // Set asset.
        pAssetPtr->setAssetId( pFieldValue );

        return;
   }

    // Warn.
    Con::warnf( "(TypeParticleAssetPtr) - Cannot set multiple args to a single asset." );
}

//------------------------------------------------------------------------------

ParticleAsset::ParticleAsset() :
                    mLifetime( 0.0f ),
                    mLifeMode( INFINITE_LIFEMODE )

{
    // Set Vector Associations.
    VECTOR_SET_ASSOCIATION( mEmitters );  

    // Initialize particle fields.
    mParticleFields.addField( mParticleLifeScale.getBase(), "LifetimeScale", 1000.0f, 0.0f, 100.0f, 1.0f );
    mParticleFields.addField( mQuantityScale.getBase(), "QuantityScale", 1000.0f, 0.0f, 100.0f, 1.0f );
    mParticleFields.addField( mSizeXScale.getBase(), "SizeXScale", 1000.0f, 0.0f, 100.0f, 1.0f );
    mParticleFields.addField( mSizeYScale.getBase(), "SizeYScale", 1000.0f, 0.0f, 100.0f, 1.0f );
    mParticleFields.addField( mSpeedScale.getBase(), "SpeedScale", 1000.0f, 0.0f, 100.0f, 1.0f );
    mParticleFields.addField( mSpinScale.getBase(), "SpinScale", 1000.0f, -100.0f, 100.0f, 1.0f );
    mParticleFields.addField( mFixedForceScale.getBase(), "FixedForceScale", 1000.0f, -100.0f, 100.0f, 1.0f  );
    mParticleFields.addField( mRandomMotionScale.getBase(), "RandomMotionScale", 1000.0f, 0.0f, 100.0f, 1.0f );
    mParticleFields.addField( mAlphaChannelScale.getBase(), "AlphaChannelScale", 1000.0f, 0.0f, 100.0f, 1.0f );
}

//------------------------------------------------------------------------------

ParticleAsset::~ParticleAsset()
{
    // Clear the emitters.
    clearEmitters();
}

//------------------------------------------------------------------------------

void ParticleAsset::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addProtectedField("Lifetime", TypeF32, Offset(mLifetime, ParticleAsset), &setLifetime, &defaultProtectedGetFn, &writeLifetime, "");
    addProtectedField("LifeMode", TypeEnum, Offset(mLifeMode, ParticleAsset), &setLifeMode, &defaultProtectedGetFn, &writeLifeMode, 1, &LifeModeTable);
}

//------------------------------------------------------------------------------

void ParticleAsset::copyTo(SimObject* object)
{
    // Fetch particle asset object.
   ParticleAsset* pParticleAsset = static_cast<ParticleAsset*>( object );

   // Sanity!
   AssertFatal( pParticleAsset != nullptr, "ParticleAsset::copyTo() - Object is not the correct type.");

   // Copy parent.
   Parent::copyTo( object );

   // Copy fields.
   pParticleAsset->setLifetime( getLifetime() );
   pParticleAsset->setLifeMode( getLifeMode() );

   // Copy particle fields.
   mParticleFields.copyTo( pParticleAsset->mParticleFields );

   // Copy the emitters.
   pParticleAsset->clearEmitters();

   for (auto pEmitter: mEmitters)
   {
       // Create a new emitter.
       ParticleAssetEmitter* pNewEmitter = new ParticleAssetEmitter();
       pParticleAsset->addEmitter( pNewEmitter );

       // Copy emitter.
       pEmitter->copyTo( pNewEmitter );
   }
}

//------------------------------------------------------------------------------

void ParticleAsset::onDeleteNotify( SimObject* object )
{
    // Fetch emitter.
    ParticleAssetEmitter* pParticleAssetEmitter = dynamic_cast<ParticleAssetEmitter*>( object );

    // Ignore if not an emitter.
    if ( pParticleAssetEmitter == nullptr )
        return;

    auto itr = std::find(mEmitters.begin(), mEmitters.end(), object);
    if (itr != mEmitters.end())
        mEmitters.erase(itr);
}

//------------------------------------------------------------------------------

bool ParticleAsset::isAssetValid( void ) const
{
    return mEmitters.size() > 0;
}

//------------------------------------------------------------------------------

void ParticleAsset::setLifetime( const F32 lifetime )
{
    // Ignore no change.
    if ( mIsEqual( lifetime, mLifetime ) )
        return;

    // Is lifetime valid?
    if ( lifetime < 0.0f )
    {
        // No, so warn.
        Con::warnf( "ParticleAsset::setLifetime() - Lifetime cannot be negative." );
        return;
    }

    mLifetime = lifetime;

    // Refresh the asset.
    refreshAsset();
}

//------------------------------------------------------------------------------

void ParticleAsset::setLifeMode( const LifeMode lifemode )
{
    // Ignore no change.
    if ( lifemode == mLifeMode )
        return;

    // Is life mode valid?
    if ( lifemode == INVALID_LIFEMODE )
    {
        // No, so warn.
        Con::warnf( "ParticleAsset::setLifeMode() - Life mode is invalid." );
        return;
    }

    mLifeMode = lifemode;

    // Refresh the asset.
    refreshAsset();
}

//------------------------------------------------------------------------------

void ParticleAsset::initializeAsset( void )
{
    // Call parent.
    Parent::initializeAsset();

    // Currently there is no specific initialization required.
}

//-----------------------------------------------------------------------------

ParticleAssetEmitter* ParticleAsset::createEmitter( void )
{
    // Create an emitter.
    ParticleAssetEmitter* pParticleAssetEmitter = new ParticleAssetEmitter();

    // Add the emitter.
    if ( addEmitter( pParticleAssetEmitter ) )
        return pParticleAssetEmitter;

    // Error.
    delete pParticleAssetEmitter;
    return nullptr;
}

//-----------------------------------------------------------------------------

bool ParticleAsset::addEmitter( ParticleAssetEmitter* pParticleAssetEmitter )
{
    // Sanity!
    AssertFatal( pParticleAssetEmitter != nullptr, "Cannot add a NULL particle asset emitter." );

    // Does the particle already have an owner?
    if ( pParticleAssetEmitter->getOwner() != nullptr )
    {
        Con::warnf( "ParticleAsset::addEmitter() - Cannot add a particle asset emitter that already has an owner." );
        return false;
    }

    // Is the emitter registered?
    if ( !pParticleAssetEmitter->isProperlyAdded() )
    {
        // No, so register it.
        if ( !pParticleAssetEmitter->registerObject() )
        {
            // Failed so warn.
            Con::warnf( "ParticleAsset::addEmitter() - Failed to register emitter." );
            return false;
        }
    }

    // Set the owner.
    pParticleAssetEmitter->setOwner( this );

    // Add the emitter.
    mEmitters.push_back( pParticleAssetEmitter );

    // Start delete notify.
    deleteNotify( pParticleAssetEmitter );

    return true;
}

//------------------------------------------------------------------------------

void ParticleAsset::removeEmitter( ParticleAssetEmitter* pParticleAssetEmitter, const bool deleteEmitter )
{
    // Sanity!
    AssertFatal( pParticleAssetEmitter != nullptr, "Cannot remove a NULL particle asset emitter." );

    // Is this emitter owned by this asset?
    if ( pParticleAssetEmitter->getOwner() != this )
    {
        // No, so warn.
        Con::warnf( "ParticleAsset::removeEmitter() - Cannot remove the particle emitter as it is not owned by this particle asset." );
        return;
    }

    // Iterate emitters.
    auto emitterItr = mEmitters.find( pParticleAssetEmitter );
    {
        if ( *emitterItr == pParticleAssetEmitter )
        {
            // Remove emitter.
            mEmitters.erase( emitterItr );

            // Remove owner.
            pParticleAssetEmitter->setOwner( nullptr );

            // Stop delete notify.
            clearNotify( pParticleAssetEmitter );

            // If requested, delete the emitter.
            if ( deleteEmitter )
                pParticleAssetEmitter->deleteObject();

            return;
        }
    }

    // Warn.
    Con::warnf( "ParticleAsset::removeEmitter() - Cannot remove the particle emitter as it is not part of this particle asset." );
}

//------------------------------------------------------------------------------

void ParticleAsset::clearEmitters( void )
{
    // Remove all emitters.
    while( mEmitters.size() > 0 )
    {
        mEmitters.back()->deleteObject();
        mEmitters.pop_back();
    }
}

//------------------------------------------------------------------------------

ParticleAssetEmitter* ParticleAsset::getEmitter( const U32 emitterIndex ) const
{
    // Is emitter index valid?
    if ( emitterIndex >= (U32)mEmitters.size() )
    {
        // No, so warn.
        Con::warnf( "ParticleAsset::getEmitter() - Invalid emitter index." );
        return nullptr;
    }

    return mEmitters[emitterIndex];
}

//------------------------------------------------------------------------------

ParticleAssetEmitter* ParticleAsset::findEmitter( const char* pEmitterName ) const
{
    // Sanity!
    AssertFatal( pEmitterName != nullptr, "ParticleAsset::findEmitter() - Cannot find a NULL emitter name." );

   // Finish if there are no emitters.
   if ( getEmitterCount() == 0 )
       return NULL;

    // Fetch emitter name.
    StringTableEntry emitterName = StringTable->insert( pEmitterName );

    // Search for emitter..
    for( ParticleAssetEmitter* emitterItr:mEmitters )
    {
        if ( emitterItr->getEmitterName() == emitterName )
            return emitterItr;
    }

    // Not found.
    return nullptr;
}

//-----------------------------------------------------------------------------

void ParticleAsset::moveEmitter( S32 fromIndex, S32 toIndex )
{
   // Check From Emitter Index.
   if ( fromIndex < 0 || fromIndex >= (S32)getEmitterCount() )
   {
      // Warn.
      Con::warnf("ParticleAsset::moveEmitter() - Invalid From-Emitter-Index (%d)", fromIndex);
      return;
   }

   // Check To Emitter Index.
   if ( toIndex < 0 || toIndex >= (S32)getEmitterCount() )
   {
      // Warn.
      Con::warnf("ParticleAsset::moveEmitter() - Invalid To-Emitter-Index (%d)", toIndex);
      return;
   }

   // We need to skip an object if we're inserting above the object.
   if ( toIndex > fromIndex )
      toIndex++;
   else
      fromIndex++;

   // Fetch Emitter to be moved.
   typeEmitterVector::iterator fromItr = (mEmitters.begin()+fromIndex);

   // Fetch Emitter to be inserted at.
   typeEmitterVector::iterator toItr = (mEmitters.begin()+toIndex);

   // Insert Object at new Position.
   mEmitters.insert( toItr, (*fromItr) );

   // Remove Original Reference.
   mEmitters.erase( fromItr );
}

//------------------------------------------------------------------------------

void ParticleAsset::onTamlCustomWrite( TamlCustomNodes& customNodes )
{
    // Debug Profiling.
    PROFILE_SCOPE(ParticleAsset_OnTamlCustomWrite);

    // Write the fields.
    mParticleFields.onTamlCustomWrite( customNodes );
}

//-----------------------------------------------------------------------------

void ParticleAsset::onTamlCustomRead( const TamlCustomNodes& customNodes )
{
    // Debug Profiling.
    PROFILE_SCOPE(ParticleAsset_OnTamlCustomRead);

    // Read the fields.
    mParticleFields.onTamlCustomRead( customNodes );
}

//-----------------------------------------------------------------------------

static void WriteCustomTamlSchema( const AbstractClassRep* pClassRep, TiXmlElement* pParentElement )
{
    // Sanity!
    AssertFatal( pClassRep != nullptr,  "ParticleAsset::WriteCustomTamlSchema() - ClassRep cannot be NULL." );
    AssertFatal( pParentElement != nullptr,  "ParticleAsset::WriteCustomTamlSchema() - Parent Element cannot be NULL." );

    // Write the particle asset fields.
    ParticleAsset particleAsset;
    particleAsset.getParticleFields().WriteCustomTamlSchema( pClassRep, pParentElement );
}

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT_CHILDREN_SCHEMA(ParticleAsset, WriteCustomTamlSchema);
