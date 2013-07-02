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



#include <unistd.h>
#include "platform/platform.h"
#include "console/console.h"
#include "string/stringTable.h"
#include "platform/platformInput.h"
#include "platform/threads/thread.h"


#pragma mark ---- Various Directories ----
//-----------------------------------------------------------------------------
const char* Platform::getUserDataDirectory() 
{
   // application support directory is most in line with the current usages of this function.
   // this may change with later usage
   // perhaps the user data directory should be pref-controlled?
   //NSString *nsDataDir = [@"~/Library/Application Support/" stringByStandardizingPath];
    //-Mat using Documents directory in same folder as .app 
   NSString *nsDataDir = [@"~/Documents/" stringByStandardizingPath];
   return StringTable->insert([nsDataDir UTF8String]);
}

//-----------------------------------------------------------------------------
const char* Platform::getUserHomeDirectory() 
{
   return StringTable->insert([[@"~/" stringByStandardizingPath] UTF8String]);
}

//-----------------------------------------------------------------------------
StringTableEntry Platform::osGetTemporaryDirectory()
{
   NSString *tdir = NSTemporaryDirectory();
   const char *path = [tdir UTF8String];
   return StringTable->insert(path);
}

#pragma mark ---- Administrator ----
//-----------------------------------------------------------------------------
/* <Mat> may need these for buttons later
#pragma mark ---- MessageBox ----
struct _NSStringMap
{
   S32 num;
   NSString* ok;
   NSString* cancel;
   NSString* third;
};

static _NSStringMap sgButtonTextMap[] =
{
   { MBOk,                 @"Ok",    nil,        nil },
   { MBOkCancel,           @"Ok",    @"Cancel",  nil },
   { MBRetryCancel,        @"Retry", @"Cancel",  nil },
   { MBSaveDontSave,       @"Save",  @"Discard", nil },
   { MBSaveDontSaveCancel, @"Save",  @"Cancel",  @"Don't Save" },
   { -1, nil, nil, nil }
};

struct _NSAlertResultMap
{
   S32 num;
   S32 ok;
   S32 cancel;
   S32 third;
};

static _NSAlertResultMap sgAlertResultMap[] = 
{
   { MBOk,                 MROk,    0,          0 },
   { MBOkCancel,           MROk,    MRCancel,   0 },
   { MBRetryCancel,        MRRetry, MRCancel,   0 },
   { MBSaveDontSave,       MROk,    MRDontSave, 0 },
   { MBSaveDontSaveCancel, MROk,    MRCancel,   MRDontSave },
   { -1, nil, nil, nil }
};
*/
//-----------------------------------------------------------------------------
S32 Platform::messageBox(const UTF8 *title, const UTF8 *message, MBButtons buttons, MBIcons icon)
{
    Platform::AlertOK( title, message );//<Mat> maybe add in a a way to do other buttons
    return 1;
}

#pragma mark ---- File IO ----
//-----------------------------------------------------------------------------
bool Platform::pathCopy(const char* source, const char* dest, bool nooverwrite)
{
   NSFileManager *manager = [NSFileManager defaultManager];
   @autoreleasepool {
   
      NSString *nsource = [@(source) stringByStandardizingPath];
      NSString *ndest   = [@(dest) stringByStandardizingPath];
      NSString *ndestFolder = [ndest stringByDeletingLastPathComponent];
      
      if(! [manager fileExistsAtPath:nsource])
      {
         Con::errorf("Platform::pathCopy: no file exists at %s",source);
         return false;
      }
       
       //Catcher for the errors.
       NSError* returnValue = nil;
      
      if( [manager fileExistsAtPath:ndest] )
      {
         if(nooverwrite)
         {
            Con::errorf("Platform::pathCopy file already exists at %s",dest);
            return false;
         }
          
         Con::warnf("Deleting files at path: %s", dest);
         bool deleted = [manager removeItemAtPath:ndest error:&returnValue];
         if(!deleted)
         {
            Con::errorf("Copy failed! Could not delete files at path: %s", dest);
            return false;
         }
      }
      
      if([manager fileExistsAtPath:ndestFolder] == NO)
      {
         ndestFolder = [ndestFolder stringByAppendingString:@"/"]; // createpath requires a trailing slash
         Platform::createPath([ndestFolder UTF8String]);
      }
      
      bool ret = [manager copyItemAtPath:nsource toPath:ndest error:&returnValue];
      
      return ret;
   }
   
}

//-----------------------------------------------------------------------------
bool Platform::fileRename(const char *source, const char *dest)
{
   if(source == NULL || dest == NULL)
      return false;
      
   NSFileManager *manager = [NSFileManager defaultManager];
   
   NSString *nsource = [manager stringWithFileSystemRepresentation:source length:dStrlen(source)];
   NSString *ndest   = [manager stringWithFileSystemRepresentation:dest length:dStrlen(dest)];
   
   if(! [manager fileExistsAtPath:nsource])
   {
      Con::errorf("Platform::fileRename: no file exists at %s",source);
      return false;
   }
   
   if( [manager fileExistsAtPath:ndest] )
   {
      Con::warnf("Platform::fileRename: Deleting files at path: %s", dest);
   }
    
    NSError* returnValue = NULL;
   
   bool ret = [manager moveItemAtPath:nsource toPath:ndest error:&returnValue];
  
   return ret;
}

//-----------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
void Platform::enableKeyboardTranslation(void)
{

}

//-----------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
void Platform::disableKeyboardTranslation(void)
{

}

