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
#include "memory/frameAllocator.h"
//#include "io/zip/zipArchive.h"
#include "io/resource/resourceManager.h"
#include "io/resource/resourceManager_ScriptBinding.h"
#include "string/findMatch.h"
#include "console/console.h"
#include "console/consoleTypes.h"

ResManager * ResManager::smResManager = nullptr;

const char *ResManager::smExcludedDirectories = ".svn;CVS";

//------------------------------------------------------------------------------
ResourceObject::ResourceObject ()
{
  lockCount = 0;
  mInstance = nullptr;
  mZipArchive = nullptr;
  mCentralDir = nullptr;
  path = StringTable->EmptyString;     ///< Resource path.
  name = StringTable->EmptyString;     ///< Resource name.
}

ResourceObject::~ResourceObject()
{
   ResourceManager->unlink(this);
}

void ResourceObject::destruct ()
{
   // If the resource was not loaded because of an error, the resource
   // pointer will be nullptr
   SAFE_DELETE(mInstance);

//   // Free if it is ResourceObject::File and is NOT ResourceObject::VolumeBlock
//   if((flags & ResourceObject::File) && !(flags & ResourceObject::VolumeBlock) )
//   {
//      // [tom, 10/26/2006] We don't want to delete if it's a volume block since
//      // the archive will be freed when the zip file resource object is freed.
//      SAFE_DELETE(mZipArchive);
//   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

ResManager::ResManager ()
{
   echoFileNames = false;
   primaryPath[0] = 0;
   writeablePath[0] = 0;
   pathList = nullptr;
   mLoggingMissingFiles = false;
//   usingVFS = false;
}

void ResManager::fileIsMissing(const char *fileName)
{
   if(mLoggingMissingFiles)
   {
      char *name = dStrdup(fileName);
      mMissingFileList.push_back(name);
   }
}

void ResManager::setMissingFileLogging(bool logging)
{
   mLoggingMissingFiles = logging;
   if(!mLoggingMissingFiles)
      clearMissingFileList();
}

void ResManager::clearMissingFileList()
{
   while(mMissingFileList.size())
   {
      dFree(mMissingFileList[0]);
      mMissingFileList.pop_front();
   }
   mMissingFileList.clear();
}

bool ResManager::getMissingFileList(std::deque<char *> &list)
{
   if(!mMissingFileList.size())
      return false;

   for(U32 i = 0; i < (U32)mMissingFileList.size();i ++)
   {
      for(U32 j = 0; j < (U32)list.size(); j++)
      {
         if(!dStrcmp(list[j], mMissingFileList[i]))
         {
            dFree(mMissingFileList[i]);
            mMissingFileList[i] = nullptr;
            break;
         }
      }
      if(mMissingFileList[i])
         list.push_back(mMissingFileList[i]);
   }

   mMissingFileList.clear();

   return true;
}

void ResourceObject::getFileTimes (FileTime * createTime, FileTime * modifyTime)
{

   if( !path || !name )
   {
      createTime = modifyTime = nullptr;
      return;
   }
      
   char buffer[1024];
   Platform::makeFullPathName(path, buffer, sizeof(buffer));
   U32 len = (U32)(dStrlen(buffer));
   dSprintf (buffer + len, sizeof (buffer) - len, "/%s", name);
   
   Platform::getFileTimes (buffer, createTime, modifyTime);
}

//------------------------------------------------------------------------------

ResManager::~ResManager ()
{
   purge ();
   // volume list should be gone.

   if (pathList)
      dFree (pathList);

   for (auto walk :resourceList )
      walk->destruct ();

   while ( !resourceList.empty() )
      freeResource (resourceList.front());

   for (RegisteredExtension *temp: registeredList)
      delete temp;
   
   registeredList.clear();
}

#ifdef TORQUE_DEBUG
void ResManager::dumpResources (const bool onlyLoaded)
{
   for (auto walk: resourceList)
      if ( !onlyLoaded || walk->mInstance != nullptr)
         Con::errorf ("Resource: %s/%s (%d)", walk->path, walk->name,
             walk->lockCount);
}
#endif

//------------------------------------------------------------------------------

void ResManager::create ()
{
   AssertFatal (ResourceManager == nullptr,
          "ResourceManager::create: manager already exists.");
   smResManager = new ResManager;

   Con::addVariable("Pref::ResourceManager::excludedDirectories", TypeString, &smExcludedDirectories);
}


//------------------------------------------------------------------------------

void ResManager::destroy ()
{
   AssertFatal (ResourceManager != nullptr,
           "ResourceManager::destroy: manager does not exist.");
   delete smResManager;
   smResManager = nullptr;
}

//------------------------------------------------------------------------------

void ResManager::setFileNameEcho (bool on)
{
   echoFileNames = on;
}

//------------------------------------------------------------------------------

bool ResManager::isValidWriteFileName (const char *fn)
{
//   if(isUsingVFS())
//      return false;

   return true;

   if (!writeablePath[0])
      return true;

   // get the path to the file
   const char * path = dStrrchr (fn, '/');
   if (!path)
      path = fn;
   else
   {
      if (!dStrchr (path, '.'))
         return false;
   }

   // now loop through the writeable path.
   const char * start = writeablePath;
   for (;;)
   {
      const char * end = dStrchr (writeablePath, ';');
      if (!end)
         end = writeablePath + dStrlen (writeablePath);

      //if (end - start == pathLen && !dStrnicmp (start, path, pathLen))
      if(dStrnicmp(start, fn, end - start) == 0)
         return true;
      if (end[0])
         start = end + 1;
      else
         break;
     }

   // now check prefs folder
   const char * prefsPath = Platform::getPrefsPath();
   if (dStrnicmp(prefsPath, fn, dStrlen(prefsPath)) == 0)
      return true;

   // now check user data folder
   const char * dataPath = Platform::getUserDataDirectory();
   if (dStrnicmp(dataPath, fn, dStrlen(dataPath)) == 0)
      return true;

   return false;
}

void ResManager::setWriteablePath (const char *path)
{
   dStrcpy (writeablePath, path);
}

//------------------------------------------------------------------------------

static const char * buildPath (StringTableEntry path, StringTableEntry file)
{
   static char buf[1024];
   if (path)
      Platform::makeFullPathName(file, buf, sizeof(buf), path);
   else
      dStrcpy (buf, file);
   return buf;
}

//------------------------------------------------------------------------------

static void getPaths (const char *fullPath, StringTableEntry & path,
   StringTableEntry & fileName)
{
   static char buf[1024];
   char *ptr = (char *) dStrrchr (fullPath, '/');
   if (!ptr)
   {
      path = nullptr;
      fileName = StringTable->insert (fullPath);
   }
   else
   {
      S32 len = S32(ptr - fullPath);
      dStrncpy (buf, fullPath, len);
      buf[len] = 0;
      fileName = StringTable->insert (ptr + 1);
      path = StringTable->insert (buf);
   }
}

//------------------------------------------------------------------------------

//bool ResManager::addVFSRoot(Zip::ZipArchive *vfs)
//{
//   ResourceObject *ro = createResource (StringTable->EmptyString, StringTable->EmptyString);
//   dictionary.pushBehind (ro, ResourceObject::File);
//
//   // [tom, 10/28/2006] Using VolumeBlock here so that destruct() doesnt try and
//   // delete the archive.
//   ro->flags = ResourceObject::VolumeBlock;
//   ro->fileOffset = 0;
//   ro->fileSize = 0;
//   ro->compressedFileSize = 0;
//   ro->mZipArchive = vfs;
//   ro->zipPath = StringTable->EmptyString;
//   ro->zipName = StringTable->EmptyString;
//
//   usingVFS = true;
//
//   return scanZip(ro);
//}

//bool ResManager::scanZip (ResourceObject * zipObject)
//{
//   const char *zipPath = buildPath(zipObject->zipPath, zipObject->zipName);
//   if(zipObject->mZipArchive == nullptr)
//   {
//      zipObject->mZipArchive = new Zip::ZipArchive;
//      if(! zipObject->mZipArchive->openArchive(zipPath))
//      {
//         SAFE_DELETE(zipObject->mZipArchive);
//         return false;
//      }
//   }
//
//   for(U32 i = 0;i < zipObject->mZipArchive->numEntries();++i)
//   {
//      const Zip::CentralDir &dir = (*zipObject->mZipArchive)[i];
//
//      // FIXME [tom, 10/26/2006] This is pretty lame
//      char buf[1024];
//      dStrncpy(buf, dir.mFilename, sizeof(buf));
//      buf[sizeof(buf)-1] = 0;
//
//      // Iterate through the string and change any
//      // characters with \\ to /
//      char* scan = buf;
//      while (*scan != '\0')
//      {
//         if (*scan == '\\')
//            *scan = '/';
//         scan++;
//      }
//
//      const char *zipFN = zipObject->mZipArchive->getFilename() ? zipObject->mZipArchive->getFilename() : "";
//
//      FrameTemp<char> zipPath(dStrlen(zipFN) + dStrlen(buf) + 2);
//      dStrcpy(zipPath, zipFN);
//
//      char* dot = dStrrchr(zipPath, '.');
//      if(dot)
//      {
//         dot -= 2;
//         dot[2] = '\0';
//         dStrcat(zipPath, "/");
//      }
//
//      dStrcat(zipPath, buf);
//
//      // Create file base name
//      char* pPathEnd = dStrrchr(zipPath, '/');
//      if(pPathEnd == nullptr)
//         continue;
//
//      pPathEnd[0] = '\0';
//      const char * path = StringTable->insert(zipPath);
//      const char * file = StringTable->insert(pPathEnd + 1);
//
//      ResourceObject *ro = createZipResource(path, file, zipObject->zipPath, zipObject->zipName);
//
//      ro->flags = ResourceObject::VolumeBlock;
//      ro->fileSize = dir.mUncompressedSize;
//      ro->compressedFileSize = dir.mCompressedSize;
//      ro->fileOffset = dir.mLocalHeadOffset;
//      ro->mZipArchive = zipObject->mZipArchive;
//      ro->mCentralDir = &dir;
//
//      dictionary.pushBehind (ro, ResourceObject::File);
//   }
//
//   return true;
//}

//------------------------------------------------------------------------------

void ResManager::searchPath (const char *path, bool noDups /* = false */, bool ignoreZips /* = false */ )
{
   AssertFatal (path != nullptr, "No path to dump?");

   // Set up exclusions.
   initExcludedDirectories();

   Vector < Platform::FileInfo > fileInfoVec;
   Platform::dumpPath (path, fileInfoVec);

   for (U32 i = 0; i < (U32)fileInfoVec.size (); i++)
   {
      Platform::FileInfo & rInfo = fileInfoVec[i];

      // Create a resource for this file...
      //
      if(noDups && dictionary.find(rInfo.pFullPath, rInfo.pFileName) != nullptr)
         continue;

      ResourceObject *ro = createResource (rInfo.pFullPath, rInfo.pFileName);
      dictionary.pushBehind (ro, ResourceObject::File);

      //Con::printf("> ResourceManager: Found file '%s' in path '%s'.", rInfo.pFileName, rInfo.pFullPath );

      ro->flags = ResourceObject::File;
      ro->fileOffset = 0;
      ro->fileSize = rInfo.fileSize;
      ro->compressedFileSize = rInfo.fileSize;

//      // see if it's a zip
//      const char *extension = dStrrchr (ro->name, '.');
//
//      if (extension && !dStricmp (extension, ".zip") && !ignoreZips )
//      {
//         // Copy the path and files names to the zips resource object
//         ro->zipName = rInfo.pFileName;
//         ro->zipPath = rInfo.pFullPath;
//         scanZip(ro);
//      }
   }

   // Clear Exclusion list
   Platform::clearExcludedDirectories();
}


//------------------------------------------------------------------------------

//bool ResManager::setModZip(const char* path)
//{
//   // Get the path and add .zip to the end of the dir
//   const char* ext =  ".zip";
//   char* modPath = new char[dStrlen(path) + dStrlen(ext) + 1]; // make enough room.
//   dStrcpy(modPath, path);
//   dStrcat(modPath, ext);
//
//   // Now we have to go through the root and look for our zipped up mod
//   // this is unfortunately necessary because there is no means to get
//   // a individual files properties -- we can only do it in one
//   // big dump
//   const char *basePath = Platform::getCurrentDirectory();
//   Vector < Platform::FileInfo > pathInfo;
//   Platform::dumpPath (basePath, pathInfo);
//   for(Platform::FileInfo file: pathInfo)
//   {
//      if(!dStricmp(file.pFileName, modPath))
//      {
//         // Setup the resource to the zip file itself
//         ResourceObject *zip = createResource(basePath, file.pFileName);
//         dictionary.pushBehind(zip, ResourceObject::File);
//         zip->flags = ResourceObject::File;
//         zip->fileOffset = 0;
//         zip->fileSize = file.fileSize;
//         zip->compressedFileSize = file.fileSize;
//         zip->zipName = file.pFileName;
//         zip->zipPath = basePath;
//
//         // Setup the resource for the zip contents
//         // ..now open the volume and add all its resources to the dictionary
//         scanZip(zip);
//
//         // Break from the loop since we got our one file
//         delete [] modPath;
//         return true;
//      }
//   }
//
//   delete [] modPath;
//   return false;
//}

//------------------------------------------------------------------------------

void ResManager::initExcludedDirectories()
{
   // Set up our excluded directories.
   Platform::clearExcludedDirectories();

   // ignored is a semi-colon delimited list of names.
   char *working = dStrdup(smExcludedDirectories);
   char* temp = dStrtok( working, ";" );
   while ( temp )
   {
      Platform::addExcludedDirectory(temp);
      temp = dStrtok( nullptr, ";" );
   }

   dFree(working);
}

void ResManager::addPath(const char *path, bool ignoreZips )
{
   searchPath(path, true, ignoreZips );
}

void ResManager::removePath(const char *path)
{
   auto rwalk = resourceList.begin();
   while (rwalk != resourceList.end())
   {
      const char *fname = buildPath((*rwalk)->path, (*rwalk)->name);
      if(!(*rwalk)->mInstance && FindMatch::isMatch(path, fname, false))
      {
         unlink(*rwalk);
         dictionary.remove (*rwalk);
         auto rtemp = *rwalk;
         rwalk++;
         freeResource (rtemp);
      }
      else
         rwalk++;
   }
}


void ResManager::setModPaths (U32 numPaths, const char **paths)
{
   // [tom, 10/28/2006] If we're using a VFS, we don't want to do this
   // since it'll remove all the stuff we've already added.
//   if(usingVFS)
//      return;

   // detach all the files.
   for(ResourceObject * pwalk: resourceList)
      pwalk->flags |= ResourceObject::Added;

   U32 pathLen = 0;

   // Set up exclusions.
   initExcludedDirectories();

   // Make sure invalid paths are not processed
   Vector<const char*> validPaths;

   // Determine if the mod paths are valid
   for (U32 i = 0; i < numPaths; i++)
   {
      pathLen += (dStrlen (paths[i]) + 1);

      // Load zip first so that local files override
//      setModZip(paths[i]);
      searchPath (paths[i]);

      // Copy this path to the validPaths list
      validPaths.push_back(paths[i]);
   }

   Platform::clearExcludedDirectories();

   if (!pathLen)
      return;

   // Build the internal path list string
   pathList = (char *) dRealloc (pathList, pathLen);
   dStrcpy (pathList, validPaths[0]);
   dsize_t strlen;
   for (U32 i = 1; i < (U32)validPaths.size(); i++)
   {
      strlen = dStrlen (pathList);
      dSprintf (pathList + strlen, pathLen - strlen, ";%s", validPaths[i]);
   }

   // Unlink all 'added' that aren't loaded.
   auto rwalk = resourceList.begin();
   while (rwalk != resourceList.end())
   {
      if (((*rwalk)->flags & ResourceObject::Added) && !(*rwalk)->mInstance)
      {
         unlink(*rwalk);
         dictionary.remove (*rwalk);
         auto rtemp = *rwalk;
         rwalk++;
         freeResource (rtemp);
      }
      else
         rwalk++;
   }
}


const char * ResManager::getModPaths ()
{
   return ((const char *) pathList);
}

//------------------------------------------------------------------------------


S32 ResManager::getSize (const char *fileName)
{
   ResourceObject * ro = find (fileName);
   if (!ro)
      return 0;
   else
      return ro->fileSize;
}

//------------------------------------------------------------------------------

const char * ResManager::getFullPath (const char *fileName, char *path, U32 pathlen)
{
   AssertFatal (fileName, "ResourceManager::getFullPath: fileName is nullptr");
   AssertFatal (path, "ResourceManager::getFullPath: path is nullptr");
   ResourceObject *obj = find (fileName);
   if (!obj)
      dStrcpy (path, fileName);
   else
      Platform::makeFullPathName(obj->name, path, pathlen, obj->path);
   return path;
}

//------------------------------------------------------------------------------

const char *ResManager::getPathOf (const char *fileName)
{
   AssertFatal (fileName, "ResourceManager::getPathOf: fileName is nullptr");
   ResourceObject *obj = find (fileName);
   if (!obj)
      return nullptr;
   else
      return obj->path;
}

//------------------------------------------------------------------------------

const char * ResManager::getModPathOf (const char *fileName)
{
   AssertFatal (fileName, "ResourceManager::getModPathOf: fileName is nullptr");

   if (!pathList)
      return nullptr;

   ResourceObject *obj = find (fileName);
   if (!obj)
      return nullptr;

   char buffer[256];
   char *base;
   const char *list = pathList;
   do
   {
      base = buffer;
      *base = 0;
      while (*list && *list != ';')
      {
         *base++ = *list++;
      }
      if (*list == ';')
         ++list;

      *base = 0;

      if (dStrncmp (buffer, obj->path, (base - buffer)) == 0)
         return StringTable->insert (buffer);
   }
   while (*list);

   return nullptr;
}

//------------------------------------------------------------------------------

const char *ResManager::getBasePath ()
{
   if (!pathList)
      return nullptr;
   const char *base = dStrrchr (pathList, ';');
   return base ? (base + 1) : pathList;
}


//------------------------------------------------------------------------------

void ResManager::registerExtension (const char *name, RESOURCE_CREATE_FN create_fn)
{
   AssertFatal (!getCreateFunction (name),
           "ResourceManager::registerExtension: file extension already registered.");

   const char *extension = dStrrchr (name, '.');
   AssertFatal (extension,
           "ResourceManager::registerExtension: file has no extension.");

   RegisteredExtension *add = new RegisteredExtension;
   add->mExtension = StringTable->insert (extension);
   add->mCreateFn = create_fn;
   
   registeredList.push_front(add);
}

//------------------------------------------------------------------------------

RESOURCE_CREATE_FN ResManager::getCreateFunction (const char *name)
{
   const char * s = dStrrchr (name, '.');
   if (!s)
      return (nullptr);

   for (auto ptr: registeredList)
   {
      if (dStricmp (s, ptr->mExtension) == 0)
         return (ptr->mCreateFn);
   }
   return (nullptr);
}


//------------------------------------------------------------------------------

void ResManager::unlock (ResourceObject * obj)
{
   if (!obj)
      return;

   AssertFatal (obj->lockCount > 0,
          "ResourceManager::unlock: lock count is zero.");

   //set the timeout to the max requested
   if (--obj->lockCount == 0)
   {
      resourceList.remove(obj);
      timeoutList.push_back(obj);
   }
}

void ResManager::unlink (ResourceObject * obj)
{
   resourceList.remove(obj);
   timeoutList.remove(obj);
}

//------------------------------------------------------------------------------
// gets the crc of the file, ignores the stream type

bool ResManager::getCrc (const char *fileName, U32 & crcVal,
   const U32 crcInitialVal)
{
   ResourceObject *obj = find (fileName);
   if (!obj)
      return (false);

   // check if in a volume
   if (obj->flags & (ResourceObject::VolumeBlock | ResourceObject::File))
   {
      // can't crc locked resources...
      if (obj->lockCount)
         return false;

      // get rid of the resource
      // have to make sure user can't have it sitting around in the resource cache

      unlink (obj);
      obj->destruct ();

       std::iostream *stream = openStream (obj);

      U32 waterMark = 0xFFFFFFFF;

      U8 *buffer;
      U32 maxSize = FrameAllocator::getHighWaterMark () - FrameAllocator::getWaterMark ();
      if (maxSize < (U32)obj->fileSize)
         buffer = new U8[obj->fileSize];
      else
      {
         waterMark = FrameAllocator::getWaterMark ();
         buffer = (U8 *) FrameAllocator::alloc (obj->fileSize);
      }

      stream->read ((char*)buffer, obj->fileSize);

      // get the crc value
      crcVal = calculateCRC (buffer, obj->fileSize, crcInitialVal);
      if (waterMark == 0xFFFFFFFF)
         delete[]buffer;
      else
         FrameAllocator::setWaterMark (waterMark);

      closeStream (stream);
      return (true);
   }

   return (false);
}

//------------------------------------------------------------------------------

ResourceObject *ResManager::load (const char *fileName, bool computeCRC)
{
   // if filename is not known, exit now
   ResourceObject *obj = find (fileName);
   if (!obj)
      return nullptr;

   // if no one has a lock on this, but it's loaded and it needs to
   // be CRC'd, delete it and reload it.
   if (!obj->lockCount && computeCRC && obj->mInstance)
      obj->destruct ();

   obj->lockCount++;
   unlink (obj);      // remove from purge list

   if (!obj->mInstance)
   {
      obj->mInstance = loadInstance (obj, computeCRC);
      if (!obj->mInstance)
      {
         obj->lockCount--;
         return nullptr;
      }
   }
   return obj;
}

//------------------------------------------------------------------------------

ResourceInstance * ResManager::loadInstance (const char *fileName, bool computeCRC)
{
   // if filename is not known, exit now
   ResourceObject *obj = find (fileName);
   if (!obj)
      return nullptr;

   return loadInstance (obj, computeCRC);
}

//------------------------------------------------------------------------------

static const char *alwaysCRCList = ".ter.dif.dts";

ResourceInstance * ResManager::loadInstance (ResourceObject * obj, bool computeCRC)
{
	std::iostream *stream = openStream (obj);
   if (!stream)
      return nullptr;

   if (!computeCRC)
   {
      const char *x = dStrrchr (obj->name, '.');
      if (x && dStrstr (alwaysCRCList, x))
         computeCRC = true;
   }

   if (computeCRC)
      obj->crc = calculateCRCStream (stream, InvalidCRC);
   else
      obj->crc = InvalidCRC;

   RESOURCE_CREATE_FN createFunction = ResourceManager->getCreateFunction (obj->name);

   if(!createFunction)
   {
       AssertWarn( false, "ResourceObject::construct: nullptr resource create function.");
       Con::errorf("ResourceObject::construct: nullptr resource create function for '%s'.", obj->name);
       return nullptr;
   }

   ResourceInstance *ret = createFunction (*stream);
   if(ret)
      ret->mSourceResource = obj;
   closeStream (stream);
   return ret;
}

//------------------------------------------------------------------------------

std::iostream * ResManager::openStream(const char *fileName)
{
   ResourceObject *obj = find (fileName);
   if (!obj)
      return nullptr;
   return openStream (obj);
}

//------------------------------------------------------------------------------

std::iostream * ResManager::openStream(ResourceObject *obj)
{
   // if filename is not known, exit now
   if (!obj)
      return nullptr;

   if (echoFileNames)
      Con::printf ("FILE ACCESS: %s/%s", obj->path, obj->name);

   // used for openStream stream access
   std::fstream *diskStream = nullptr;

   // if disk file
   if (obj->flags & (ResourceObject::File))
   {
      diskStream = new std::fstream(buildPath (obj->path, obj->name), std::fstream::in | std::ios::binary);
      if( !diskStream  )
         return nullptr;

       diskStream->seekg (0, diskStream->end);
       auto length = diskStream->tellg();
       diskStream->seekg (0, diskStream->beg);

       obj->fileSize = (S32)length;
      return diskStream;
   }

   // if zip file

//   if (obj->flags & ResourceObject::VolumeBlock)
//   {
//      AssertFatal(obj->mZipArchive, "mZipArchive is nullptr");
//      AssertFatal(obj->mCentralDir, "mCentralDir is nullptr");
//
//      return obj->mZipArchive->openFileForRead(obj->mCentralDir);
//   }

   // unknown type
   return nullptr;
}

//------------------------------------------------------------------------------

void ResManager::closeStream(std::iostream *stream)
{
   // FIXME [tom, 10/26/2006] Note that this should really hand off to ZipArchive if it's
   // a zip stream, but there's currently no way to get the ZipArchive pointer from
   // here so we just repeat the relevant code. This is pretty lame.

//   FilterStream *currentStream, *nextStream;
//
//   // Try to cast the stream to a FilterStream
//   nextStream = dynamic_cast<FilterStream*>(stream);
//   bool isFilter = nextStream != nullptr;
//
//   // While the nextStream is valid (meaning it was successfully cast to a FilterStream)
//   while (nextStream)
//   {
//      // Point currentStream to nextStream
//      currentStream = nextStream;
//      // Point stream to the Stream contained within the current FilterStream
//      stream = currentStream->getStream();
//      // Detach the current FilterStream from the Stream contained within it
//      currentStream->detachStream();
//      // Try to cast the stream (which was already contained within a FilterStream) to a FilterStream
//      nextStream = dynamic_cast<FilterStream*>(stream);
//      // Delete the FilterStream that was wrapping stream
//      delete currentStream;
//   }
//
//   if(! isFilter)
      delete stream;
}

//------------------------------------------------------------------------------

ResourceObject *ResManager::find (const char *fileName)
{
   if (!fileName)
      return nullptr;
   StringTableEntry path, file;
   getPaths (fileName, path, file);
   ResourceObject *ret = dictionary.find (path, file);
   if(!ret)
   {
      // If we couldn't find the file in the resource list (generated
      // by setting the modPaths) then try to load it directly
      if (Platform::isFile(fileName))
      {
         ret = createResource (path, file);
         dictionary.insert(ret, path, file);
//         dictionary.pushBehind (ret, ResourceObject::File);

         ret->flags = ResourceObject::File;
         ret->fileOffset = 0;

         S32 fileSize = Platform::getFileSize(fileName);
         ret->fileSize = fileSize;
         ret->compressedFileSize = fileSize;

         return ret;
      }

      fileIsMissing(fileName);
   }
   return ret;
}

//------------------------------------------------------------------------------

ResourceObject *ResManager::find (const char *fileName, U32 flags)
{
   if (!fileName)
      return nullptr;
   StringTableEntry path, file;
   getPaths (fileName, path, file);
   return dictionary.find (path, file, flags);
}


//------------------------------------------------------------------------------
// Add resource constructed outside the manager

bool ResManager::add (const char *name, ResourceInstance * addInstance,
   bool extraLock)
{
   StringTableEntry path, file;
   getPaths (name, path, file);

   ResourceObject *obj = dictionary.find (path, file);
   if (obj && obj->mInstance)
      // Resource already exists?
      return false;

   if (!obj)
      obj = createResource (path, file);

   dictionary.pushBehind (obj,
          ResourceObject::File | ResourceObject::VolumeBlock);
   obj->mInstance = addInstance;
   addInstance->mSourceResource = obj;
   obj->lockCount = extraLock ? 2 : 1;
   unlock (obj);
   return true;
}

//------------------------------------------------------------------------------

void ResManager::purge ()
{
    while ( !timeoutList.empty() )
    {
        ResourceObject *temp = timeoutList.front();
        unlink(temp);
        if (temp->flags & ResourceObject::Added)
            freeResource (temp);
    }
}


//------------------------------------------------------------------------------

void ResManager::purge (ResourceObject * obj)
{
   AssertFatal (obj->lockCount == 0,
                "ResourceManager::purge: handle lock count is not ZERO.");

//   obj->unlink ();
   obj->destruct ();
}

//------------------------------------------------------------------------------
// serialize sorts a list of files by .zip and position within the zip
// it allows an aggregate (material list, etc) to find the preferred
// loading order for a set of files.
//------------------------------------------------------------------------------

struct ResourceObjectIndex
{
   ResourceObject *ro;
   const char *fileName;

//   static S32 QSORT_CALLBACK compare (const void *s1, const void *s2)
//   {
//      const ResourceObjectIndex *r1 = (ResourceObjectIndex *) s1;
//      const ResourceObjectIndex *r2 = (ResourceObjectIndex *) s2;
//
//      if (r1->ro->path != r2->ro->path)
//         return r1->ro->path - r2->ro->path;
//      if (r1->ro->name != r2->ro->name)
//         return r1->ro->name - r2->ro->name;
//      return r1->ro->fileOffset - r2->ro->fileOffset;
//   }


    static S32 QSORT_CALLBACK compare (const ResourceObjectIndex r1, const ResourceObjectIndex r2)
    {
        if (r1.ro->path != r2.ro->path)
            return r1.ro->path < r2.ro->path;
        if (r1.ro->name != r2.ro->name)
            return r1.ro->name < r2.ro->name;
        return r1.ro->fileOffset < r2.ro->fileOffset;
    }
};

//------------------------------------------------------------------------------

void ResManager::serialize (Vector < const char *>&filenames)
{
   Vector < ResourceObjectIndex > sortVector;

   sortVector.reserve(filenames.size ());

   U32 i;
   for (i = 0; i < (U32)filenames.size(); i++)
   {
      ResourceObjectIndex roi;
      roi.ro = find(filenames[i]);
      roi.fileName = filenames[i];
      sortVector.push_back (roi);
   }

//   dQsort ((void *)sortVector.address(), sortVector.size (), sizeof (ResourceObjectIndex), ResourceObjectIndex::compare);
    std::sort(sortVector.begin(), sortVector.end(), ResourceObjectIndex::compare);

   for (i = 0; i < (U32)filenames.size (); i++)
      filenames[i] = sortVector[i].fileName;
}

//------------------------------------------------------------------------------

ResourceObject * ResManager::findMatch (const char *expression, const char **fn, ResourceObject* start)
{
   std::list<ResourceObject*>::iterator startItr = resourceList.begin();

   if (start)
   {
      startItr = std::find(resourceList.begin(), resourceList.end(), start);
      if (startItr == resourceList.end())
         return nullptr;
   }

   while (startItr != resourceList.end())
   {
       const char *fname = buildPath ((*startItr)->path, (*startItr)->name);

      if (FindMatch::isMatch (expression, fname, false))
      {
         *fn = fname;
         return *startItr;
      }
      startItr++;
   }
   return nullptr;
}

ResourceObject * ResManager::findMatchMultiExprs (const char *multiExpression, const char **fn,
      ResourceObject * start)
{
   std::list<ResourceObject*>::iterator startItr = resourceList.begin();
   
   if (start)
   {
      startItr = std::find(resourceList.begin(), resourceList.end(), start);
      if (startItr == resourceList.end())
         return nullptr;
   }
   
   while (startItr != resourceList.end())
   {
      const char *fname = buildPath ((*startItr)->path, (*startItr)->name);

      if (FindMatch::isMatchMultipleExprs(multiExpression, fname, false))
      {
         *fn = fname;
         return *(startItr);
      }
      startItr++;
   }
   return nullptr;
}

S32 ResManager::findMatches (FindMatch * pFM)
{
   static char buffer[16384];
   S32 bufl = 0;
   for (auto walk = resourceList.begin(); walk != resourceList.end() && !pFM->isFull (); walk++)
   {
      const char * fpath = buildPath ((*walk)->path, (*walk)->name);
      if (bufl + dStrlen (fpath) >= 16380)
         return pFM->numMatches ();
      dStrcpy (buffer + bufl, fpath);
      if (pFM->findMatch (buffer + bufl))
         bufl += dStrlen (fpath) + 1;
   }
   return (pFM->numMatches ());
}

//------------------------------------------------------------------------------

bool ResManager::findFile (const char *name)
{
  return (bool) find (name);
}

//------------------------------------------------------------------------------

ResourceObject * ResManager::createResource (StringTableEntry path, StringTableEntry file)
{
   ResourceObject *newRO = dictionary.find (path, file);
   if (newRO)
      return newRO;

   newRO = new ResourceObject;
   newRO->path = path;
   newRO->name = file;
   newRO->lockCount = 0;
   newRO->mInstance = nullptr;
   newRO->flags = ResourceObject::Added;
   dictionary.insert (newRO, path, file);
   newRO->fileSize = newRO->fileOffset = newRO->compressedFileSize = 0;
   newRO->zipPath = nullptr;
   newRO->zipName = nullptr;
   newRO->crc = InvalidCRC;

   return newRO;
}

//------------------------------------------------------------------------------

//ResourceObject * ResManager::createZipResource (StringTableEntry path, StringTableEntry file,
//   StringTableEntry zipPath,
//   StringTableEntry zipName)
//{
//   ResourceObject *newRO = dictionary.find (path, file, zipPath, zipName);
//   if (newRO)
//      return newRO;
//
//   newRO = new ResourceObject;
//   newRO->path = path;
//   newRO->name = file;
//   newRO->lockCount = 0;
//   newRO->mInstance = nullptr;
//   newRO->flags = ResourceObject::Added;
//   dictionary.insert (newRO, path, file);
//   newRO->fileSize = newRO->fileOffset = newRO->compressedFileSize = 0;
//   newRO->zipPath = zipPath;
//   newRO->zipName = zipName;
//   newRO->crc = InvalidCRC;
//   newRO->mZipArchive = nullptr;
//   newRO->mCentralDir = nullptr;
//
//   return newRO;
//}

//------------------------------------------------------------------------------

void ResManager::freeResource (ResourceObject * ro)
{
   unlink(ro);
   ro->destruct ();
   dictionary.remove (ro);
   delete ro;
}

//------------------------------------------------------------------------------

bool ResManager::openFileForWrite(std::fstream &stream, const char *fileName, std::fstream::openmode accessMode)
{
   //if (!isValidWriteFileName (fileName))
      //return false;

   // tag it on to the first directory
   char path[1024];
   dStrcpy (path, fileName);
   char *file = dStrrchr (path, '/');
   if (!file)
      return false;      // don't allow storing files in root
   *file++ = 0;

   if (!Platform::createPath (fileName))   // create directory tree
      return false;
   stream.open (fileName, accessMode);
   if (stream.bad())
      return false;

   // create a resource for the file.
   ResourceObject *ro = createResource (StringTable->insert (path), StringTable->insert (file));
   ro->flags = ResourceObject::File;
   ro->fileOffset = 0;
   ro->fileSize = 0;
   ro->compressedFileSize = 0;
   return true;
}

//ConsoleFunction(isUsingVFS, bool, 1, 1, "()\n"
//                "@return Returns true if using Virtual File System")
//{
//   return ResourceManager->isUsingVFS();
//}
