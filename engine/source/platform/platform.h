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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdlib.h>
#include "torqueConfig.h"
#include "platform/types.h"
#include "platform/platformAssert.h"
#include "platform/nativeDialogs/msgBox.h"
#include "platform/platformEndian.h"
#include "platform/platformCPU.h"
#include "platform/platformString.h"
#include "platform/platformNetwork.h"
#include "platform/platformMemory.h"
#include "platform/platformFont.h"
#include "platform/platformMath.h"
#include "string/str.h"
#include "memory/safeDelete.h"

//------------------------------------------------------------------------------

template <class T> class Vector;
class Point2I;
class GFXWindowTarget;

//------------------------------------------------------------------------------

struct Platform
{
    struct LocalTime
    {
        U8  sec;        // seconds after minute (0-59)
        U8  min;        // Minutes after hour (0-59)
        U8  hour;       // Hours after midnight (0-23)
        U8  month;      // Month (0-11; 0=january)
        U8  monthday;   // Day of the month (1-31)
        U8  weekday;    // Day of the week (0-6, 6=sunday)
        U16 year;       // current year minus 1900
        U16 yearday;    // Day of year (0-365)
        bool isdst;     // true if daylight savings time is active
    };

    struct FileInfo
    {
        const char* pFullPath;
        const char* pFileName;
        U32 fileSize;

        bool equal( const FileInfo& fileInfo )
        {
            return
                fileInfo.pFullPath == pFullPath &&
                fileInfo.pFileName == pFileName &&
                fileInfo.fileSize == fileSize;
        }
    };

    struct VolumeInformation
    {
        StringTableEntry  RootPath;
        StringTableEntry  Name;
        StringTableEntry  FileSystem;
        U32               SerialNumber;
        U32               Type;
        bool              ReadOnly;
    };

    typedef void* FILE_HANDLE;
    enum DFILE_STATUS
    {
        DFILE_OK = 1
    };


    /// Application.
    static void init();
    static void initConsole();
    static void process();
    static void shutdown();
    static void sleep(U32 ms);
    static void restartInstance();
    static void postQuitMessage(const U32 in_quitVal);
    static void forceShutdown(S32 returnValue);

    /// User.
    static StringTableEntry getUserHomeDirectory();
    static StringTableEntry getUserDataDirectory();

    // Window state
    void setWindowLocked(bool locked);
    void minimizeWindow();
    void setWindowSize( U32 newWidth, U32 newHeight, bool fullScreen );
    void closeWindow();

    /// GUI.
    static void AlertOK(const char *windowTitle, const char *message);
    static bool AlertOKCancel(const char *windowTitle, const char *message);
    static bool AlertRetry(const char *windowTitle, const char *message);
    static bool AlertYesNo(const char *windowTitle, const char *message);
    static S32 messageBox(const UTF8 *title, const UTF8 *message, MBButtons buttons = MBOkCancel, MBIcons icon = MIInformation);

    /// Input.
    static void enableKeyboardTranslation(void);
    static void disableKeyboardTranslation(void);

    /// Date & Time.
    static U32 getTime( void );
    static U32 getVirtualMilliseconds( void );
    static U32 getRealMilliseconds( void );
    static void advanceTime(U32 delta);
    static S32 getBackgroundSleepTime();
    static void getLocalTime(LocalTime &);
    static S32 compareFileTimes(const FileTime &a, const FileTime &b);

    /// Math.
    static float getRandom();

    /// Debug.
    static void debugBreak();
    static void outputDebugString(const char *string);
    static void cprintf(const char* str);

    /// File IO.
    static StringTableEntry getCurrentDirectory();
    static bool setCurrentDirectory(StringTableEntry newDir);
    static StringTableEntry getTemporaryDirectory();
    static StringTableEntry getTemporaryFileName();
    static StringTableEntry getExecutableName();
    static StringTableEntry getExecutablePath(); 
    static void setMainDotCsDir(const char *dir);
    static StringTableEntry getMainDotCsDir();
    static StringTableEntry getPrefsPath(const char *file = NULL);
    static char *makeFullPathName(const char *path, char *buffer, U32 size, const char *cwd = NULL);
    static StringTableEntry stripBasePath(const char *path);
    static bool isFullPath(const char *path);
    static StringTableEntry makeRelativePathName(const char *path, const char *to);
    static bool dumpPath(const char *in_pBasePath, Vector<FileInfo>& out_rFileVector, S32 recurseDepth = -1);
    static bool dumpDirectories( const char *path, Vector<StringTableEntry> &directoryVector, S32 depth = 0, bool noBasePath = false );
    static bool hasSubDirectory( const char *pPath );
    static bool getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime);
    static bool isFile(const char *pFilePath);
    static S32  getFileSize(const char *pFilePath);
    static bool hasExtension(const char* pFilename, const char* pExtension);
    static bool isDirectory(const char *pDirPath);
    static bool isSubDirectory(const char *pParent, const char *pDir);
    static void addExcludedDirectory(const char *pDir);
    static void clearExcludedDirectories();
    static bool isExcludedDirectory(const char *pDir);
    static bool createPath(const char *path);
    static bool deleteDirectory( const char* pPath );
    static bool fileDelete(const char *name);
    static bool fileRename(const char *oldName, const char *newName);
    static bool fileTouch(const char *name);
    static bool pathCopy(const char *fromName, const char *toName, bool nooverwrite = true);
    static StringTableEntry osGetTemporaryDirectory();

    static bool deleteDirectoryRecursive( const char* pPath );

    /// Misc.
    static StringTableEntry createUUID( void );
    static bool openWebBrowser( const char* webAddress );
    static void openFolder( const char* path );
    static const char* getClipboard();
    static bool setClipboard(const char *text);
};

#endif // _PLATFORM_H_
