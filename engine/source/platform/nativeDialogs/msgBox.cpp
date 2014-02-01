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

#include "platform/platform.h"
#include "console/console.h"
#include "platform/nativeDialogs/msgBox.h"
#include "string/stringTable.h"

// these are the return values for message box dialog buttons
void initMessageBoxVars()
{
   Con::setIntVariable("$MROk",        MROk);
   Con::setIntVariable("$MRCancel",    MRCancel);
   Con::setIntVariable("$MRRetry",     MRRetry);
   Con::setIntVariable("$MRDontSave",  MRDontSave);
}

//////////////////////////////////////////////////////////////////////////

static EnumTable::Enums sgButtonEnums[] =
{
   EnumTable::Enums( MBOk,                 "Ok" ),
   EnumTable::Enums( MBOkCancel,           "OkCancel" ),
   EnumTable::Enums( MBRetryCancel,        "RetryCancel" ),
   EnumTable::Enums( MBSaveDontSave,       "SaveDontSave" ), // maps to yes/no on win, to save/discard on mac.
   EnumTable::Enums( MBSaveDontSaveCancel, "SaveDontSaveCancel" ), // maps to yes/no/cancel on win, to save/cancel/don'tsave on mac.
   EnumTable::Enums( 0, StringTable->EmptyString )
};

static EnumTable::Enums sgIconEnums[] =
{
   EnumTable::Enums( MIInformation,        "Information" ),// win: blue i, mac: app icon or talking head
   EnumTable::Enums( MIWarning,            "Warning" ),    // win & mac: yellow triangle with exclamation pt
   EnumTable::Enums( MIStop,               "Stop" ),       // win: red x, mac: app icon or stop icon, depending on version
   EnumTable::Enums( MIQuestion,           "Question" ),   // win: blue ?, mac: app icon
   EnumTable::Enums( 0, StringTable->EmptyString )
};

//////////////////////////////////////////////////////////////////////////

static S32 getIDFromName(EnumTable::Enums *table, const char *name, S32 def = -1)
{
   for(S32 i = 0; table[i].second != StringTable->EmptyString; ++i)
   {
      if( table[i].second.compare(name) == 0)
         return table[i].first;
   }

   AssertWarn(false,"getIDFromName(): didn't find that name" );
   return def;
}

//-------------------------------------------------------------------------

#include "msgBox_ScriptBinding.h"

