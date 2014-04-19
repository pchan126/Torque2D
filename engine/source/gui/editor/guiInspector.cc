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
#include "gui/editor/guiInspector.h"
#include "gui/editor/guiInspector_ScriptBinding.h"
#include "gui/buttons/guiIconButtonCtrl.h"
#include "memory/frameAllocator.h"
#include "graphics/gfxDrawUtil.h"

//////////////////////////////////////////////////////////////////////////
// GuiInspector
//////////////////////////////////////////////////////////////////////////
// The GuiInspector Control houses the body of the inspector.
// It is not exposed as a conobject because it merely does the grunt work
// and is only meant to be used when housed by a scroll control.  Therefore
// the GuiInspector control is a scroll control that creates it's own 
// content.  That content being of course, the GuiInspector control.
IMPLEMENT_CONOBJECT(GuiInspector);

GuiInspector::GuiInspector()
{
   mGroups.clear();
   mTarget = nullptr;
   mPadding = 1;
}

GuiInspector::~GuiInspector()
{
   clearGroups();
}

bool GuiInspector::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   return true;
}

//////////////////////////////////////////////////////////////////////////
// Handle Parent Sizing (We constrain ourself to our parents width)
//////////////////////////////////////////////////////////////////////////
void GuiInspector::parentResized(const RectI &oldParentRect, const RectI &newParentRect)
{
   GuiControl *parent = getParent();
   if( parent && dynamic_cast<GuiScrollCtrl*>(parent) != nullptr )
   {
      GuiScrollCtrl *scroll = dynamic_cast<GuiScrollCtrl*>(parent);
      setWidth( ( newParentRect.extent.x - ( scroll->scrollBarThickness() + 4  ) ) );
   }
   else
      Parent::parentResized(oldParentRect,newParentRect);
}

bool GuiInspector::findExistentGroup( StringTableEntry groupName )
{
   // If we have no groups, it couldn't possibly exist
   if( mGroups.empty() )
      return false;

   // Attempt to find it in the group list
   for( GuiInspectorGroup* i:mGroups )
   {
      if( dStricmp( i->getGroupName(), groupName ) == 0 )
         return true;
   }

   return false;
}

void GuiInspector::clearGroups()
{
   // If we're clearing the groups, we want to clear our target too.
   mTarget = nullptr;

   // If we have no groups, there's nothing to clear!
   if( mGroups.empty() )
      return;

   // Attempt to find it in the group list
   for( GuiInspectorGroup *i:mGroups )
      if( i->isProperlyAdded() )
         i->deleteObject();

   mGroups.clear();
}

void GuiInspector::inspectObject( SimObject *object )
{  
   GuiCanvas  *guiCanvas = getRoot();
   if( !guiCanvas )
      return;

   SimObjectPtr<GuiControl> currResponder = guiCanvas->getFirstResponder();

   // If our target is the same as our current target, just update the groups.
   if( mTarget == object )
   {
      for ( GuiInspectorGroup *i:mGroups )
         i->inspectGroup();

      // Don't steal first responder
      if( !currResponder.isNull() )
         guiCanvas->setFirstResponder( currResponder );

      return;
   }

   // Clear our current groups
   clearGroups();

   // Set Target
   mTarget = object;

   // Always create the 'general' group (for un-grouped fields)
   GuiInspectorGroup* general = new GuiInspectorGroup( mTarget, "General", this );
   if( general != nullptr )
   {
      general->registerObject();
      mGroups.push_back( general );
      addObject( general );
   }

   // Grab this objects field list
   AbstractClassRep::FieldList &fieldList = mTarget->getModifiableFieldList();
   AbstractClassRep::FieldList::iterator itr;

   // Iterate through, identifying the groups and create necessary GuiInspectorGroups
   for(itr = fieldList.begin(); itr != fieldList.end(); itr++)
   {
      if(itr->type == AbstractClassRep::StartGroupFieldType && !findExistentGroup( itr->pGroupname ) )
      {
         GuiInspectorGroup *group = new GuiInspectorGroup( mTarget, itr->pGroupname, this );
         if( group != nullptr )
         {
            group->registerObject();
            mGroups.push_back( group );
            addObject( group );
         }            
      }
   }

   // Deal with dynamic fields
   GuiInspectorGroup *dynGroup = new GuiInspectorDynamicGroup( mTarget, "Dynamic Fields", this);
   if( dynGroup != nullptr )
   {
      dynGroup->registerObject();
      mGroups.push_back( dynGroup );
      addObject( dynGroup );
   }

   // If the general group is still empty at this point, kill it.
   for(S32 i=0; i<mGroups.size(); i++)
   {
      if(mGroups[i] == general && general->mStack->empty())
      {
         mGroups.erase(i);
         general->deleteObject();
         updatePanes();
      }
   }

   // Don't steal first responder
   if( !currResponder.isNull() )
      guiCanvas->setFirstResponder( currResponder );

}


void GuiInspector::setName( const char* newName )
{
   if( mTarget == nullptr )
      return;

   // Only assign a new name if we provide one
   mTarget->assignName(newName);

}


//////////////////////////////////////////////////////////////////////////
// GuiInspectorField
//////////////////////////////////////////////////////////////////////////
// The GuiInspectorField control is a representation of a single abstract
// field for a given ConsoleObject derived object.  It handles creation
// getting and setting of it's fields data and editing control.  
//
// Creation of custom edit controls is done through this class and is
// dependent upon the dynamic console type, which may be defined to be
// custom for different types.
//
// Note : GuiInspectorField controls must have a GuiInspectorGroup as their
//        parent.  
IMPLEMENT_CONOBJECT(GuiInspectorField);

// Caption width is in percentage of total width
S32 GuiInspectorField::smCaptionWidth = 50;

GuiInspectorField::GuiInspectorField( GuiInspectorGroup* parent, SimObjectPtr<SimObject> target, AbstractClassRep::Field* field )
{
   if( field != nullptr )
      mCaption    = StringTable->insert( field->pFieldname );
   else
      mCaption    = StringTable->EmptyString;

   mParent     = parent;
   mTarget     = target;
   mField      = field;
   mFieldArrayIndex = nullptr;
   setBounds(0,0,100,18);

}

GuiInspectorField::GuiInspectorField()
{
   mCaption       = StringTable->EmptyString;
   mParent        = nullptr;
   mTarget        = nullptr;
   mField         = nullptr;
   mFieldArrayIndex = nullptr;
   setBounds(0,0,100,18);
}

GuiInspectorField::~GuiInspectorField()
{
}

//////////////////////////////////////////////////////////////////////////
// Get/Set Data Functions
//////////////////////////////////////////////////////////////////////////
void GuiInspectorField::setData( const char* data )
{
   if( mField == nullptr || mTarget == nullptr )
      return;

   mTarget->inspectPreApply();

   mTarget->setDataField( mField->pFieldname, mFieldArrayIndex, data );

   // Force our edit to update
   updateValue( data );

   mTarget->inspectPostApply();
}

const char* GuiInspectorField::getData()
{
   if( mField == nullptr || mTarget == nullptr )
      return "";

   return mTarget->getDataField( mField->pFieldname, mFieldArrayIndex );
}

void GuiInspectorField::setInspectorField( AbstractClassRep::Field *field, const char*arrayIndex ) 
{
   mField = field; 

   if( arrayIndex != nullptr )
   {
      mFieldArrayIndex = StringTable->insert( arrayIndex );

      auto frameTempSize = dStrlen( field->pFieldname ) + 32;
      FrameTemp<char> valCopy( frameTempSize );
      dSprintf( (char *)valCopy, frameTempSize, "%s%s", field->pFieldname, arrayIndex );

      mCaption = StringTable->insert( valCopy ); 
   }
   else
      mCaption = StringTable->insert( field->pFieldname );
}


StringTableEntry GuiInspectorField::getFieldName() 
{ 
   // Sanity
   if ( mField == nullptr )
      return StringTable->EmptyString;

   // Array element?
   if( mFieldArrayIndex != nullptr )
   {
      auto frameTempSize = dStrlen( mField->pFieldname ) + 32;
      FrameTemp<char> valCopy( frameTempSize );
      dSprintf( (char *)valCopy, frameTempSize, "%s%s", mField->pFieldname, mFieldArrayIndex );

      // Return formatted element
      return StringTable->insert( valCopy );
   }

   // Plain ole field name.
   return mField->pFieldname; 
};
//////////////////////////////////////////////////////////////////////////
// Overrideables for custom edit fields
//////////////////////////////////////////////////////////////////////////
GuiControl* GuiInspectorField::constructEditControl()
{
   GuiControl* retCtrl = new GuiTextEditCtrl();
   
   // If we couldn't construct the control, bail!
   if( retCtrl == nullptr )
      return retCtrl;

   // Let's make it look pretty.
   retCtrl->setField( "profile", "GuiInspectorTextEditProfile" );

   // Don't forget to register ourselves
   registerEditControl( retCtrl );

   char szBuffer[512];
   dSprintf( szBuffer, 512, "%d.apply(%d.getText());",getId(), retCtrl->getId() );
   retCtrl->setField("AltCommand", szBuffer );
   retCtrl->setField("Validate", szBuffer );


   return retCtrl;
}

void GuiInspectorField::registerEditControl( GuiControl *ctrl )
{
   if(!mTarget)
      return;
        
   char szName[512];
   dSprintf( szName, 512, "IE_%s_%d_%s_Field", ctrl->getClassName(), mTarget->getId(),mCaption);

   // Register the object
   ctrl->registerObject( szName );
}

void GuiInspectorField::onRender(Point2I offset, const RectI &updateRect)
{
   if(mCaption && mCaption[0])
   {
      // Calculate Caption Rect
      RectI captionRect( offset , Point2I((S32) mFloor( getWidth() * (F32)( (F32)GuiInspectorField::smCaptionWidth / 100.0f ) ), (S32)getHeight() ) );
      // Calculate Y Offset to center vertically the caption
      U32 captionYOffset = (U32)mFloor( (F32)( captionRect.extent.y - mProfile->mFont->getHeight() ) / 2 );

      RectI clipRect = GFX->getClipRect();

      if( clipRect.intersect( captionRect ) )
      {
         // Backup Bitmap Modulation
         ColorI currColor;
         GFX->getDrawUtil()->getBitmapModulation( &currColor );

         GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );

         GFX->setClipRect( RectI( clipRect.point, Point2I( captionRect.extent.x, clipRect.extent.y ) ));
         // Draw Caption ( Vertically Centered )
         U32 textY = captionRect.point.y + captionYOffset;
         U32 textX = captionRect.point.x + captionRect.extent.x - mProfile->mFont->getStrWidth(mCaption) - 6;
         Point2I textPT(textX, textY);

         GFX->getDrawUtil()->drawText( mProfile->mFont, textPT, mCaption, &mProfile->mFontColor );

         GFX->getDrawUtil()->setBitmapModulation( currColor );

         GFX->setClipRect( clipRect );
      }
   }

   Parent::onRender( offset, updateRect );
}

bool GuiInspectorField::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   if( !mTarget )
      return false;

   mEdit = constructEditControl();

   if( mEdit == nullptr )
      return false;

   // Add our edit as a child
   addObject( mEdit );

   // Calculate Caption Rect
   RectI captionRect( getPosition() , Point2I( (S32)mFloor( getWidth() * (F32)( (F32)GuiInspectorField::smCaptionWidth / 100.0 ) ), (S32)getHeight() ) );

   // Calculate Edit Field Rect
   RectI editFieldRect( Point2I( captionRect.extent.x + 1, 1 ) , Point2I( getWidth() - ( captionRect.extent.x + 5 ) , getHeight() - 1) );

   // Resize to fit properly in allotted space
   mEdit->resize( editFieldRect.point, editFieldRect.extent );

   // Prefer GuiInspectorFieldProfile
   setField( "profile", "GuiInspectorFieldProfile" );

   // Force our editField to set it's value
   updateValue( getData() );

   return true;
}

void GuiInspectorField::updateValue( const char* newValue )
{
   GuiTextEditCtrl *ctrl = dynamic_cast<GuiTextEditCtrl*>( mEdit );
   if( ctrl != nullptr )
      ctrl->setText( newValue );
}

bool GuiInspectorField::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   if (!Parent::resize( newPosition, newExtent ))
       return false;

   if( mEdit != nullptr )
   {
      // Calculate Caption Rect
      RectI captionRect( getPosition() , Point2I( (S32)mFloor( getWidth() * (F32)( (F32)GuiInspectorField::smCaptionWidth / 100.0f ) ), (S32)getHeight() ) );

      // Calculate Edit Field Rect
      RectI editFieldRect( Point2I( captionRect.extent.x + 1, 1 ) , Point2I( getWidth() - ( captionRect.extent.x + 5 ) , getHeight() - 1) );

      mEdit->resize( editFieldRect.point, editFieldRect.extent );
   }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GuiInspectorGroup
//////////////////////////////////////////////////////////////////////////
//
// The GuiInspectorGroup control is a helper control that the inspector
// makes use of which houses a collapsible pane type control for separating
// inspected objects fields into groups.  The content of the inspector is 
// made up of zero or more GuiInspectorGroup controls inside of a GuiStackControl
//
//
//
IMPLEMENT_CONOBJECT(GuiInspectorGroup);

GuiInspectorGroup::GuiInspectorGroup()
{
   setBounds(0,0,200,20);

   mChildren.clear();

   mTarget              = nullptr;
   mParent              = nullptr;
    setCanSave(false);
   // Make sure we receive our ticks.
   setProcessTicks();
}

GuiInspectorGroup::GuiInspectorGroup( SimObjectPtr<SimObject> target, StringTableEntry groupName, SimObjectPtr<GuiInspector> parent )
{
   setBounds(0,0,200,20);

   mChildren.clear();

   mCaption             = StringTable->insert(groupName);
   mTarget              = target;
   mParent              = parent;
    setCanSave(false);
}

GuiInspectorGroup::~GuiInspectorGroup()
{
   if( !mChildren.empty() )
   {
      Vector<GuiInspectorField*>::iterator i = mChildren.begin();
      for( ; i != mChildren.end(); i++ );
         
   } 
}

//////////////////////////////////////////////////////////////////////////
// t2dScene Events
//////////////////////////////////////////////////////////////////////////
bool GuiInspectorGroup::onAdd()
{
   setField( "profile", "GuiInspectorGroupProfile" );

   if( !Parent::onAdd() )
      return false;

   // Create our inner controls. Allow subclasses to provide other content.
   if(!createContent())
      return false;

   inspectGroup();

   return true;
}

bool GuiInspectorGroup::createContent()
{
   // Create our field stack control
   mStack = new GuiStackControl();
   if( !mStack )
      return false;

   // Prefer GuiTransperantProfile for the stack.
   mStack->setField( "profile", "GuiTransparentProfile" );
   mStack->registerObject();

   addObject( mStack );
   mStack->setField( "padding", "0" );

   return true;
}

//////////////////////////////////////////////////////////////////////////
// Control Sizing Animation Functions
//////////////////////////////////////////////////////////////////////////
void GuiInspectorGroup::animateToContents()
{
   calculateHeights();
   if(!empty())
      animateTo( mExpanded.extent.y );
   else
      animateTo( mHeader.extent.y );
}

GuiInspectorField* GuiInspectorGroup::constructField( S32 fieldType )
{
   ConsoleBaseType *cbt = ConsoleBaseType::getType(fieldType);
   AssertFatal(cbt, "GuiInspectorGroup::constructField - could not resolve field type!");
   assert(cbt != nullptr);

   // Alright, is it a datablock?
   if(cbt->isDatablock())
   {
      // This is fairly straightforward to deal with.
      GuiInspectorDatablockField *dbFieldClass = new GuiInspectorDatablockField( cbt->getTypeClassName() );
      if( dbFieldClass != nullptr )
      {
         // return our new datablock field with correct datablock type enumeration info
         return dbFieldClass;
      }
   }

   // Nope, not a datablock. So maybe it has a valid inspector field override we can use?
   if(!cbt->getInspectorFieldType())
      // Nothing, so bail.
      return nullptr;

   // Otherwise try to make it!
   ConsoleObject *co = create(cbt->getInspectorFieldType());
   GuiInspectorField *gif = dynamic_cast<GuiInspectorField*>(co);

   if(!gif)
   {
      // Wasn't appropriate type, bail.
      delete co;
      return nullptr;
   }

   return gif;
}

GuiInspectorField *GuiInspectorGroup::findField( StringTableEntry fieldName )
{
   // If we don't have any field children we can't very well find one then can we?
   if( mChildren.empty() )
      return nullptr;

   for( GuiInspectorField* i:mChildren )
   {
      if( i->getFieldName() != nullptr && dStricmp( i->getFieldName(), fieldName ) == 0 )
         return i;
   }

   return nullptr;
}

bool GuiInspectorGroup::inspectGroup()
{
   // We can't inspect a group without a target!
   if( !mTarget )
      return false;

   // to prevent crazy resizing, we'll just freeze our stack for a sec..
   mStack->freeze(true);

   bool bNoGroup = false;

   // Un-grouped fields are all sorted into the 'general' group
   if ( dStricmp( mCaption, "General" ) == 0 )
      bNoGroup = true;

   AbstractClassRep::FieldList &fieldList = mTarget->getModifiableFieldList();
   AbstractClassRep::FieldList::iterator itr;

   bool bGrabItems = false;
   bool bNewItems = false;

   for(itr = fieldList.begin(); itr != fieldList.end(); itr++)
   {
      if( itr->type == AbstractClassRep::StartGroupFieldType )
      {
         // If we're dealing with general fields, always set grabItems to true (to skip them)
         if( bNoGroup == true )
            bGrabItems = true;
         else if( itr->pGroupname != nullptr && dStricmp( itr->pGroupname, mCaption ) == 0 )
            bGrabItems = true;
         continue;
      }
      else if ( itr->type == AbstractClassRep::EndGroupFieldType )
      {
         // If we're dealing with general fields, always set grabItems to false (to grab them)
         if( bNoGroup == true )
            bGrabItems = false;
         else if( itr->pGroupname != nullptr && dStricmp( itr->pGroupname, mCaption ) == 0 )
            bGrabItems = false;
         continue;
      }

      if( ( bGrabItems == true || ( bNoGroup == true && bGrabItems == false ) ) && itr->type != AbstractClassRep::DeprecatedFieldType )
      {
         if( bNoGroup == true && bGrabItems == true )
            continue; 
           // This is weird, but it should work for now. - JDD
           // We are going to check to see if this item is an array
           // if so, we're going to construct a field for each array element
         if( itr->elementCount > 1 )
         {
            for(auto nI = 0; nI < itr->elementCount; nI++)
            {
               FrameTemp<char> intToStr( 64 );
               dSprintf( intToStr, 64, "%d", nI );

               const char *val = mTarget->getDataField( itr->pFieldname, intToStr );
               if (!val)
                  val = StringTable->EmptyString;


               // Copy Val and construct proper ValueName[nI] format 
               //      which is "ValueName0" for index 0, etc.
               auto frameTempSize = dStrlen( val ) + 32;
               FrameTemp<char> valCopy( frameTempSize );
               dSprintf( (char *)valCopy, frameTempSize, "%s%d", itr->pFieldname, nI );

               // If the field already exists, just update it
               GuiInspectorField *field = findField( valCopy );
               if( field != nullptr )
               {
                  field->updateValue( field->getData() );
                  continue;
               }

               bNewItems = true;

               field = constructField( itr->type );
               if( field == nullptr )
               {
                  field = new GuiInspectorField( this, mTarget, &(*itr) );
                  field->setInspectorField( &(*itr), intToStr );
               }
               else
               {
                  field->setTarget( mTarget );
                  field->setParent( this );
                  field->setInspectorField( &(*itr), intToStr );
               }

               field->registerObject();
               mChildren.push_back( field );
               mStack->addObject( field );
            }
         }
         else
         {
            // If the field already exists, just update it
            GuiInspectorField *field = findField( itr->pFieldname );
            if( field != nullptr )
            {
               field->updateValue( field->getData() );
               continue;
            }

            bNewItems = true;

            field = constructField( itr->type );
            if( field == nullptr )
               field = new GuiInspectorField( this, mTarget, &(*itr) );
            else
            {
               field->setTarget( mTarget );
               field->setParent( this );
               field->setInspectorField( &(*itr) );
            }

            field->registerObject();
            mChildren.push_back( field );
            mStack->addObject( field );

         }       
      }
   }
   mStack->freeze(false);
   mStack->updatePanes();

   // If we've no new items, there's no need to resize anything!
   if( bNewItems == false && !mChildren.empty() )
      return true;

   sizeToContents();

   setUpdate();

   return true;
}


IMPLEMENT_CONOBJECT(GuiInspectorDynamicGroup);

//////////////////////////////////////////////////////////////////////////
// GuiInspectorDynamicGroup - add custom controls
//////////////////////////////////////////////////////////////////////////
bool GuiInspectorDynamicGroup::createContent()
{
   if(!Parent::createContent())
      return false;

   // add a button that lets us add new dynamic fields.
   GuiIconButtonCtrl* addFieldBtn = new GuiIconButtonCtrl();
   {
      addFieldBtn->setBitmap("tools/gui/images/iconAdd");

      SimObject* profilePtr = Sim::findObject("EditorButton");
      if( profilePtr != nullptr )
         addFieldBtn->setControlProfile( dynamic_cast<GuiControlProfile*>(profilePtr) );

      char commandBuf[64];
      dSprintf(commandBuf, 64, "%d.addDynamicField();", this->getId());
      addFieldBtn->setField("command", commandBuf);
      addFieldBtn->setSizing(horizResizeLeft,vertResizeCenter);
      //addFieldBtn->setField("buttonMargin", "2 2");
      addFieldBtn->resize(Point2I(getWidth() - 20,2), Point2I(16, 16));
      addFieldBtn->registerObject("zAddButton");
   }

   // encapsulate the button in a dummy control.
   GuiControl* shell = new GuiControl();
   shell->setField( "profile", "GuiTransparentProfile" );
   shell->registerObject();

   shell->resize(Point2I(0,0), Point2I(getWidth(), 28));
   shell->addObject(addFieldBtn);

   // save off the shell control, so we can push it to the bottom of the stack in inspectGroup()
   mAddCtrl = shell;
   mStack->addObject(shell);

   return true;
}

//static S32 QSORT_CALLBACK compareEntries(const void* a,const void* b)
//{
//   SimFieldDictionary::Entry *fa = *((SimFieldDictionary::Entry **)a);
//   SimFieldDictionary::Entry *fb = *((SimFieldDictionary::Entry **)b);
//   return dStricmp(fa->slotName, fb->slotName);
//}

static bool compareEntries(const SimFieldDictionary::Entry *fa,const SimFieldDictionary::Entry *fb)
{
    return dStricmp(fa->slotName, fb->slotName) == -1;
}


//////////////////////////////////////////////////////////////////////////
// GuiInspectorDynamicGroup - inspectGroup override
//////////////////////////////////////////////////////////////////////////
bool GuiInspectorDynamicGroup::inspectGroup()
{
   // We can't inspect a group without a target!
   if( !mTarget )
      return false;

   // Clearing the fields and recreating them will more than likely be more
   // efficient than looking up existent fields, updating them, and then iterating
   // over existent fields and making sure they still exist, if not, deleting them.
   clearFields();
   
   // Create a vector of the fields
   Vector<SimFieldDictionary::Entry *> flist;

   // Then populate with fields
   for(auto ditr: *mTarget->getFieldDictionary())
      flist.push_back(ditr.second);

//   dQsort(flist.address(),flist.size(),sizeof(SimFieldDictionary::Entry *),compareEntries);
    std::sort(flist.begin(),flist.end(),compareEntries);

   for(auto entry: flist)
   {
      GuiInspectorField *field = new GuiInspectorDynamicField( this, mTarget, entry );
      if( field != nullptr )
      {
         field->registerObject();
         mChildren.push_back( field );
         mStack->addObject( field );
      }
   }
   
   mStack->pushObjectToBack(mAddCtrl);

   setUpdate();

   return true;
}

void GuiInspectorDynamicGroup::clearFields()
{
   // save mAddCtrl
   Sim::getGuiGroup()->addObject(mAddCtrl);
   // delete everything else
   mStack->clear();
   // clear the mChildren list.
   mChildren.clear();
   // and restore.
   mStack->addObject(mAddCtrl);
}

SimFieldDictionary::Entry* GuiInspectorDynamicGroup::findDynamicFieldInDictionary( StringTableEntry fieldName )
{
   if( !mTarget )
      return nullptr;

   SimFieldDictionary * fieldDictionary = mTarget->getFieldDictionary();

   for(SimFieldDictionary::Iterator ditr = fieldDictionary->begin(); ditr != fieldDictionary->end(); ++ditr)
   {
      SimFieldDictionary::Entry * entry = ditr->second;
      
      if( dStricmp( entry->slotName, fieldName ) == 0 )
         return entry;
   }

   return nullptr;
}

void GuiInspectorDynamicGroup::addDynamicField()
{
   // We can't add a field without a target
   if( !mTarget || !mStack )
   {
      Con::warnf("GuiInspectorDynamicGroup::addDynamicField - no target SimObject to add a dynamic field to.");
      return;
   }

   // find a field name that is not in use. 
   // But we wont try more than 100 times to find an available field.
   U32 uid = 1;
   char buf[64] = "dynamicField";
   SimFieldDictionary::Entry* entry = findDynamicFieldInDictionary(buf);
   while(entry != nullptr && uid < 100)
   {
      dSprintf(buf, sizeof(buf), "dynamicField%03d", uid++);
      entry = findDynamicFieldInDictionary(buf);
   }

   //Con::evaluatef( "%d.%s = \"defaultValue\";", mTarget->getId(), buf );
   mTarget->setDataField(StringTable->insert(buf), nullptr, "defaultValue");

   // now we simply re-inspect the object, to see the new field.
   this->inspectGroup();
   animateToContents();
}


//////////////////////////////////////////////////////////////////////////
// GuiInspectorDynamicField - Child class of GuiInspectorField 
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CONOBJECT(GuiInspectorDynamicField);

GuiInspectorDynamicField::GuiInspectorDynamicField( GuiInspectorGroup* parent, SimObjectPtr<SimObject> target, SimFieldDictionary::Entry* field )
{
   mCaption    = nullptr;

   mParent     = parent;
   mTarget     = target;
   mDynField   = field;
   setBounds(0,0,100,20);
   mRenameCtrl = nullptr;
}

void GuiInspectorDynamicField::setData( const char* data )
{
   if( mTarget == nullptr || mDynField == nullptr )
      return;

   char buf[1024];
   const char * newValue = mEdit->getScriptValue();
   dStrcpy( buf, newValue ? newValue : "" );
   collapseEscape(buf);

   mTarget->getFieldDictionary()->setFieldValue(mDynField->slotName, buf);

   // Force our edit to update
   updateValue( data );

}

const char* GuiInspectorDynamicField::getData()
{
   if( mTarget == nullptr || mDynField == nullptr )
      return "";

   return mTarget->getFieldDictionary()->getFieldValue( mDynField->slotName );
}

void GuiInspectorDynamicField::renameField( StringTableEntry newFieldName )
{
   if( mTarget == nullptr || mDynField == nullptr || mParent == nullptr || mEdit == nullptr )
   {
      Con::warnf("GuiInspectorDynamicField::renameField - No target object or dynamic field data found!" );
      return;
   }

   if( !newFieldName )
   {
      Con::warnf("GuiInspectorDynamicField::renameField - Invalid field name specified!" );
      return;
   }

   // Only proceed if the name has changed
   if( dStricmp( newFieldName, getFieldName() ) == 0 )
      return;

   // Grab a pointer to our parent and cast it to GuiInspectorDynamicGroup
   GuiInspectorDynamicGroup *group = dynamic_cast<GuiInspectorDynamicGroup*>(mParent);

   if( group == nullptr )
   {
      Con::warnf("GuiInspectorDynamicField::renameField - Unable to locate GuiInspectorDynamicGroup parent!" );
      return;
   }

   // Grab our current dynamic field value
   const char* currentValue = getData();

   // Create our new field with the value of our old field and the new fields name!
   mTarget->setDataField( newFieldName, nullptr, currentValue );

   // Configure our field to grab data from the new dynamic field
   SimFieldDictionary::Entry *newEntry = group->findDynamicFieldInDictionary( newFieldName );

   if( newEntry == nullptr )
   {
      Con::warnf("GuiInspectorDynamicField::renameField - Unable to find new field!" );
      return;
   }

   // Set our old fields data to "" (which will effectively erase the field)
   mTarget->setDataField( getFieldName(), nullptr, "" );
   
   // Assign our dynamic field pointer (where we retrieve field information from) to our new field pointer
   mDynField = newEntry;

   // Lastly we need to reassign our Command and AltCommand fields for our value edit control
   char szBuffer[512];
   dSprintf( szBuffer, 512, "%d.%s = %d.getText();",mTarget->getId(), getFieldName(), mEdit->getId() );
   mEdit->setField("AltCommand", szBuffer );
   mEdit->setField("Validate", szBuffer );
}

bool GuiInspectorDynamicField::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   mRenameCtrl = constructRenameControl();
   pushObjectToBack(mEdit);

   return true;
}

GuiControl* GuiInspectorDynamicField::constructRenameControl()
{
   // Create our renaming field
   GuiControl* retCtrl = new GuiTextEditCtrl();

   // If we couldn't construct the control, bail!
   if( retCtrl == nullptr )
      return retCtrl;

   // Let's make it look pretty.
   retCtrl->setField( "profile", "GuiInspectorTextEditRightProfile" );

   // Don't forget to register ourselves
   char szName[512];
   dSprintf( szName, 512, "IE_%s_%d_%s_Rename", retCtrl->getClassName(), mTarget->getId(), getFieldName() );
   retCtrl->registerObject( szName );


   // Our command will evaluate to :
   //
   //    if( (editCtrl).getText() !$= "" )
   //       (field).renameField((editCtrl).getText());
   //
   char szBuffer[512];
   dSprintf( szBuffer, 512, "if( %d.getText() !$= \"\" ) %d.renameField(%d.getText());",retCtrl->getId(), getId(), retCtrl->getId() );
   dynamic_cast<GuiTextEditCtrl*>(retCtrl)->setText( getFieldName() );
   retCtrl->setField("AltCommand", szBuffer );
   retCtrl->setField("Validate", szBuffer );

   // Calculate Caption Rect (Adjust for 16 pixel wide delete button)
   RectI captionRect( Point2I(getPosition().x,0) , Point2I( (S32)mFloor( getWidth() * (F32)( (F32)GuiInspectorField::smCaptionWidth / 100.0f ) ), (S32)getHeight() ) );
   RectI valueRect(mEdit->getPosition(), mEdit->getExtent() - Point2I(20, 0));
   RectI deleteRect( Point2I( getPosition().x + getWidth() - 20,2), Point2I( 16, getHeight() - 4));
   addObject( retCtrl );

   // Resize the name control to fit in our caption rect (tricksy!)
   retCtrl->resize( captionRect.point, captionRect.extent );
   // resize the value control to leave space for the delete button
   mEdit->resize(valueRect.point, valueRect.extent);

   // Finally, add a delete button for this field
   GuiIconButtonCtrl * delButt = new GuiIconButtonCtrl();
   if( delButt != nullptr )
   {
      dSprintf(szBuffer, 512, "%d.%s = \"\";%d.inspectGroup();", mTarget->getId(), getFieldName(), mParent->getId());

      delButt->setField("Bitmap", "^modules/gui/images/iconDelete");
      delButt->setField("Text", "X");
      delButt->setField("Command", szBuffer);
      delButt->setSizing(horizResizeLeft,vertResizeCenter);
      delButt->registerObject();

      delButt->resize( deleteRect.point,deleteRect.extent);

      addObject(delButt);
   }

   return retCtrl;
}

bool GuiInspectorDynamicField::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   if (!Parent::resize( newPosition, newExtent ))
       return false;

   // If we don't have a field rename control, bail!
   if( mRenameCtrl == nullptr )
      return true;

   // Calculate Caption Rect
   RectI captionRect( Point2I(getPosition().x,0) , Point2I( (S32)mFloor( getWidth() * (F32)( (F32)GuiInspectorField::smCaptionWidth / 100.0f ) ), (S32)getHeight() ) );
   RectI valueRect(mEdit->getPosition(), mEdit->getExtent() - Point2I(20, 0));

   // Resize the edit control to fit in our caption rect (tricksy!)
   mRenameCtrl->resize( captionRect.point, captionRect.extent );
   mEdit->resize( valueRect.point, valueRect.extent);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GuiInspectorDatablockField 
// Field construction for datablock types
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CONOBJECT(GuiInspectorDatablockField);

//static S32 QSORT_CALLBACK stringCompare(const void *a,const void *b)
//{
//   StringTableEntry sa = *(StringTableEntry*)a;
//   StringTableEntry sb = *(StringTableEntry*)b;
//   return(dStricmp(sb, sa));
//}

static bool stringCompare(const StringTableEntry sa,const StringTableEntry sb)
{
    return (dStricmp(sb, sa) == -1);
}

GuiInspectorDatablockField::GuiInspectorDatablockField( StringTableEntry className )
{
   setClassName(className);
};

void GuiInspectorDatablockField::setClassName( StringTableEntry className )
{
   // Walk the ACR list and find a matching class if any.
   AbstractClassRep *walk = AbstractClassRep::getClassList();
   while(walk)
   {
      if(!dStricmp(walk->getClassName(), className))
      {
         // Match!
         mDesiredClass = walk;
         return;
      }

      walk = walk->getNextClass();
   }

   // No dice.
   Con::warnf("GuiInspectorDatablockField::setClassName - no class '%s' found!", className);
   return;
}

GuiControl* GuiInspectorDatablockField::constructEditControl()
{
   GuiControl* retCtrl = new GuiPopUpMenuCtrl();

   // If we couldn't construct the control, bail!
   if( retCtrl == nullptr )
      return retCtrl;

   GuiPopUpMenuCtrl *menu = dynamic_cast<GuiPopUpMenuCtrl*>(retCtrl);

   // Let's make it look pretty.
   retCtrl->setField( "profile", "InspectorTypeEnumProfile" );

   menu->setField("text", getData());

   registerEditControl( retCtrl );

   // Configure it to update our value when the popup is closed
   char szBuffer[512];
   dSprintf( szBuffer, 512, "%d.%s = %d.getText();%d.inspect(%d);",mTarget->getId(), mField->pFieldname, menu->getId(), mParent->mParent->getId(), mTarget->getId() );
   menu->setField("Command", szBuffer );

   Vector<StringTableEntry> entries;

   SimDataBlockGroup * grp = Sim::getDataBlockGroup();
   for(SimDataBlockGroup::iterator i = grp->begin(); i != grp->end(); i++)
   {
      SimDataBlock * datablock = dynamic_cast<SimDataBlock*>(*i);

      // Skip non-datablocks if we somehow encounter them.
      if(!datablock)
         continue;

      // Ok, now we have to figure inheritance info.
      if( datablock && datablock->getClassRep()->isClass(mDesiredClass) )
         entries.push_back(datablock->getName());
   }

   // sort the entries
//   dQsort(entries.address(), entries.size(), sizeof(StringTableEntry), stringCompare);
    std::sort(entries.begin(), entries.end(), stringCompare);

   // add them to our enum
   for (auto entry:entries)
      menu->addEntry(entry, 0);

   return retCtrl;
}
