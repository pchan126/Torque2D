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

#include "game/defaultGame.h"
#include "platform/types.h"
#include "platform/Tickable.h"
#include "platform/platform.h"
#include "platform/platformInput.h"
#include "platform/platformAudio.h"
#include "platform/event.h"
#include "game/gameInterface.h"
#include "collection/vector.h"
#include "math/mMath.h"
#include "graphics/gfxDevice.h"
#include "graphics/gBitmap.h"
//#include "ts/tsShape.h"
#include "io/resource/resourceManager.h"
#include "io/fileStream.h"
#include "graphics/gfxInit.h"
#include "graphics/gfxTextureManager.h"
#include "console/console.h"
#include "sim/simBase.h"
#include "gui/guiCanvas.h"
#include "input/actionMap.h"
#include "network/connectionProtocol.h"
#include "io/bitStream.h"
#include "network/telnetConsole.h"
#include "debug/telnetDebugger.h"
#include "console/consoleTypes.h"
#include "math/mathTypes.h"
#include "graphics/gfxTextureManager.h"
#include "io/resource/resourceManager.h"
#include "network/netStringTable.h"
#include "memory/frameAllocator.h"
#include "game/version.h"
#include "debug/profiler.h"
#include "network/serverQuery.h"
#include "game/defaultGame.h"
#include "platform/nativeDialogs/msgBox.h"
#include "platform/nativeDialogs/fileDialog.h"
#include "memory/safeDelete.h"
#include "delegates/process.h"

#include <stdio.h>

#ifndef _NETWORK_PROCESS_LIST_H_
#include "network/networkProcessList.h"
#endif

#ifndef _REMOTE_DEBUGGER_BRIDGE_H_
#include "debug/remote/RemoteDebuggerBridge.h"
#endif

#ifndef _MODULE_MANAGER_H
#include "module/moduleManager.h"
#endif

#ifndef _ASSET_MANAGER_H_
#include "assets/assetManager.h"
#endif

#ifndef _PARTICLE_SYSTEM_H_
#include "2d/core/particleSystem.h"
#endif

#ifdef TORQUE_OS_IOS
#include "platformiOS/iOSProfiler.h"
#endif

// Script binding.
#include "platform/platform_ScriptBinding.h"

//-----------------------------------------------------------------------------

DefaultGame GameObject;
DemoNetInterface GameNetInterface;
NetworkProcessList gServerProcessList(true);
StringTableEntry gMasterAddress;

static F32 gTimeScale = 1.0;
static U32 gTimeAdvance = 0;
static U32 gFrameSkip = 0;

// Reset frames stats.
static F32 framePeriod = 0.0f;
static F32 frameTotalTime = 0.0f;
static F32 frameTotalLastTime = 0.0f;
static U32 frameTotalCount = 0;

//-----------------------------------------------------------------------------

bool initializeLibraries()
{
    PlatformAssert::create();
    Con::init();
    Sim::init();
    GFXInit::init();
    Process::init();

    if(!Net::init())
    {
        printf("Network Error : Unable to initialize the network... aborting.");
        return false;
    }

    gMasterAddress = StringTable->EmptyString;
    Con::addVariable( "MasterServerAddress", TypeString, &gMasterAddress );

    // Create the stock colors.
    StockColor::create();
    
#ifdef TORQUE_OS_IOS
   //3MB default is way too big for iPhone!!!
#ifdef	TORQUE_SHIPPING
    FrameAllocator::init(256 * 1024);	//256KB for now... but let's test and see!
#else
    FrameAllocator::init(512 * 1024);	//512KB for now... but let's test and see!
#endif	//TORQUE_SHIPPING
#else
    FrameAllocator::init(3 << 20);      // 3 meg frame allocator buffer
#endif	//TORQUE_OS_IOS

    ResManager::create();

    // Register known file types here
    ResourceManager->registerExtension(".jpg", constructBitmapJPEG);
    ResourceManager->registerExtension(".jpeg", constructBitmapJPEG);
    ResourceManager->registerExtension(".png", constructBitmapPNG);
    ResourceManager->registerExtension(".uft", constructNewFont);
    ResourceManager->registerExtension(".fnt", constructBMFont);

#ifdef TORQUE_OS_IOS
    ResourceManager->registerExtension(".pvr", constructBitmapPVR);
#endif
//	ResourceManager->registerExtension(".dts", constructShape);
   
    Platform::initConsole();
    NetStringTable::create();
   
    TelnetConsole::create();
    TelnetDebugger::create();

    Processor::init();
    Math::init();

    Platform::init();    // platform specific initialization

    // Initialize the particle system.
    ParticleSystem::Init();
    
#if defined(TORQUE_OS_IOS) && defined(_USE_STORE_KIT)
    storeInit();
#endif // TORQUE_OS_IOS && _USE_STORE_KIT
   
    return true;
}

//--------------------------------------------------------------------------

void shutdownLibraries()
{
    // Purge any resources on the timeout list...
    if (ResourceManager)
        ResourceManager->purge();

    TelnetDebugger::destroy();
    TelnetConsole::destroy();

    Sim::shutdown();
    Platform::shutdown();

    NetStringTable::destroy();
    Con::shutdown();

    ResManager::destroy();

    // Destroy the stock colors.
    StockColor::destroy();

    _StringTable::destroy();

    // asserts should be destroyed LAST
    FrameAllocator::destroy();

    PlatformAssert::destroy();
    Net::shutdown();

    Process::shutdown();

    // Destroy the particle system.
    ParticleSystem::destroy();
  
#ifdef _USE_STORE_KIT
    storeCleanup();
#endif // _USE_STORE_KIT
}

//--------------------------------------------------------------------------

bool initializeGame(int argc, const char **argv)
{
    Con::addVariable("timeScale", TypeF32, &gTimeScale);
    Con::addVariable("timeAdvance", TypeS32, &gTimeAdvance);
    Con::addVariable("frameSkip", TypeS32, &gFrameSkip);

    initMessageBoxVars();


    // Register the module manager.
    ModuleDatabase.registerObject( "ModuleDatabase" );

    // Register the asset database.
    AssetDatabase.registerObject( "AssetDatabase" );

    // Register the asset database as a module listener.
    ModuleDatabase.addListener( &AssetDatabase );

    ActionMap* globalMap = new ActionMap;
    globalMap->registerObject("GlobalActionMap");
    Sim::getActiveActionMapSet()->pushObject(globalMap);

    // Let the remote debugger process the command-line.
    RemoteDebuggerBridge::processCommandLine( argc, argv );

    if(argc > 2 && dStricmp(argv[1], "-project") == 0)
    {
        char playerPath[1024];
        Platform::makeFullPathName(argv[2], playerPath, sizeof(playerPath));
        Platform::setCurrentDirectory(playerPath);

        argv += 2;
        argc -= 2;
    }

    const char* defaultScriptName = "main.cs";
    bool useDefaultScript = true;

    // Change to the directory that contains main.cs
    Platform::setCurrentDirectory(Platform::getMainDotCsDir());

    // Scan executable location and all sub-directories.
    ResourceManager->setWriteablePath(Platform::getCurrentDirectory());
    ResourceManager->addPath( Platform::getCurrentDirectory() );

    FileStream scriptFileStream; 
    Stream* scriptStream;

    // Check if any command-line parameters were passed (the first is just the app name).
    if (argc > 1)
    {
        // If so, check if the first parameter is a file to open.
        if ( (scriptFileStream.open(argv[1], FileStream::Read)) && dStrncmp(argv[1], "", 1) )
        {
            // If it opens, we assume it is the script to run.
            useDefaultScript = false;
            scriptStream = &scriptFileStream;
        }
    }

    if (useDefaultScript)
    {
        bool success = false;
            success = scriptFileStream.open(defaultScriptName, FileStream::Read);

        if( !success )
        {
            char msg[1024];
            dSprintf(msg, sizeof(msg), "Failed to open \"%s\".", defaultScriptName);
            printf(" Error : %s", msg);
            return false;
        }

            scriptStream = &scriptFileStream;
    }

    // Create a script buffer.
    const U32 size = scriptStream->getStreamSize();
    char* pScriptBuffer = new char[size + 1];

    // Read script.
    scriptStream->read(size, pScriptBuffer);

    scriptFileStream.close();

    pScriptBuffer[size] = 0;

    char buffer[1024], *ptr;
    Platform::makeFullPathName(useDefaultScript ? defaultScriptName : argv[1], buffer, sizeof(buffer), Platform::getCurrentDirectory());
    ptr = dStrrchr( buffer, '/' );
    if ( ptr )
        *ptr = 0;
    Platform::setMainDotCsDir(buffer);
    Platform::setCurrentDirectory(buffer);

    const S32 errorHash = Con::getIntVariable("$ScriptErrorHash");
    Con::evaluate(pScriptBuffer, false, useDefaultScript ? defaultScriptName : argv[1]); 
    delete[] pScriptBuffer;

    // Did an error occur?
    if ( errorHash != Con::getIntVariable("$ScriptErrorHash") )
    {
        printf( "Quitting as an error occurred parsing the root script '%s'.", useDefaultScript ? defaultScriptName : argv[1] );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------

void shutdownGame()
{
    // Perform pre-exit callback.
    if( Con::isFunction("onPreExit") )
        Con::executef(1, "onPreExit");

    // Perform the exit callback.
    if( Con::isFunction("onExit") )
        Con::executef(1, "onExit");

    // Unregister the module database.
    ModuleDatabase.unregisterObject();

    // Unregister the asset database.
    AssetDatabase.unregisterObject();
}

//--------------------------------------------------------------------------

bool DefaultGame::mainInitialize(int argc, const char **argv)
{
    if(!initializeLibraries())
        return false;
    
    // Allow the window manager to process command line inputs; this is
    // done to let web plugin functionality happen in a fairly transparent way.
    PlatformWindowManager::get()->processCmdLineArgs(argc, argv);

    // Set up the command line args for the console scripts...
    Con::setIntVariable("$GameProject::argc", argc);
    U32 i;
    for (i = 0; i < (U32)argc; i++)
        Con::setVariable(avar("$GameProject::argv%d", i), argv[i]);
    if (initializeGame(argc, argv) == false)
    { 
        //Using printf cos Con:: is not around here.
        printf("\nApplication failed to start! Make sure your resources are in the correct place.");
        shutdownGame();
        shutdownLibraries();
        return false;
    }

    // Start processing ticks.
    setProcessTicks( true );

    
#ifdef TORQUE_OS_IOS	
    
    // Torque 2D does not have true, GameKit networking support. 
    // The old socket network code is untested, undocumented and likely broken. 
    // This will eventually be replaced with GameKit. 
    // For now, it is confusing to even have a checkbox in the editor that no one uses or understands. 
    // If you are one of the few that uses this, just replace the false; with the commented line. -MP 1.5

    //-Mat this is a bit of a hack, but if we don't want the network, we shut it off now. 
    // We can't do it until we've run the entry script, otherwise the script variable will not have ben loaded
    bool usesNet = false; //dAtob( Con::getVariable( "$pref::iOS::UseNetwork" ) );
    if( !usesNet ) {
        Net::shutdown();
    }
    
#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerProfilerInit();
#endif
    #endif

   return true;
}



//--------------------------------------------------------------------------

void DefaultGame::processTick( void )
{
    Con::setVariable( "Sim::Time", avar("%4.1f", (F32)Platform::getVirtualMilliseconds() / 1000.0f ) );

    // Update the frame variables periodically.
    static F32 lastFrameUpdate = frameTotalTime;    
    if ( (frameTotalTime - lastFrameUpdate) > 0.25f )
    {
        Con::setVariable( "fps::framePeriod", avar("%4.1f", 1.0f / framePeriod) );
        Con::setVariable( "fps::frameCount", avar("%u", frameTotalCount) );
        lastFrameUpdate = frameTotalTime;
    }
}

//--------------------------------------------------------------------------

void DefaultGame::advanceTime( F32 timeDelta )
{
    // Update total frame time.
    frameTotalTime += timeDelta;    

    // Update frame total count.
    frameTotalCount++;

    // Have we already processed a single frame?
    if ( frameTotalCount > 1 )
    {
        // Yes, so set the time-bias to use.
        const F32 timeBias = 0.01f;

        // Calculate the current frame period.
        framePeriod = framePeriod * ( 1.0f - timeBias ) + ( frameTotalTime - frameTotalLastTime ) * timeBias;
    }

    // Update last total frame time.
    frameTotalLastTime = frameTotalTime;
}

//--------------------------------------------------------------------------

void DefaultGame::mainLoop( double elapsedTime )
{	
#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerStart("MAIN_LOOP");
#endif	
         PROFILE_START(MainLoop);
#ifdef TORQUE_ALLOW_JOURNALING
         PROFILE_START(JournalMain);
   Game->journalProcess();
         PROFILE_END();
#endif // TORQUE_ALLOW_JOURNALING
         PROFILE_START(NetProcessMain);
   Net::process();      // read in all events
         PROFILE_END();
         PROFILE_START(PlatformProcessMain);
    Platform::process(); // keys, etc.
         PROFILE_END();
         PROFILE_START(TelconsoleProcessMain);
   TelConsole->process();
         PROFILE_END();
         PROFILE_START(TelDebuggerProcessMain);
   TelDebugger->process();
         PROFILE_END();
         PROFILE_START(TimeManagerProcessMain);
   TimeManager::process( elapsedTime); // guaranteed to produce an event
         PROFILE_END();
         PROFILE_START(GameProcessEvents);
    Game->processEvents(); // process all non-sim posted events.
         PROFILE_END();
         PROFILE_END();
    
#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerEnd("MAIN_LOOP");
    if(iPhoneProfilerGetCount() >= 60){
        iPhoneProfilerPrintAllResults();
        iPhoneProfilerProfilerInit();
    }
#endif
}

//-----------------------------------------------------------------------------

void DefaultGame::mainShutdown( void )
{
    // Stop processing ticks.
    setProcessTicks( false );

    shutdownGame();
    shutdownLibraries();

    if( Game->requiresRestart() )
    Platform::restartInstance();
}

//--------------------------------------------------------------------------

void DefaultGame::gameReactivate( void )
{
   if ( !Input::isEnabled() )
      Input::enable();

   if ( !Input::isActive() )
      Input::reactivate();

//   TextureManager::mDGLRender = true;
//   if ( Canvas )
//      Canvas->resetUpdateRegions();
}

//--------------------------------------------------------------------------

void DefaultGame::gameDeactivate( const bool noRender )
{
   if ( Input::isActive() )
      Input::deactivate();

   if ( Input::isEnabled() )
      Input::disable();

//   if ( noRender )
//      TextureManager::mDGLRender = false;
}

//--------------------------------------------------------------------------

void DefaultGame::textureKill()
{
//    TextureManager::killManager();
}

//--------------------------------------------------------------------------

void DefaultGame::textureResurrect()
{
//    TextureManager::resurrectManager();
}

//--------------------------------------------------------------------------

void DefaultGame::refreshWindow()
{
//   if(Canvas)
//      Canvas->resetUpdateRegions();
}

//--------------------------------------------------------------------------

void DefaultGame::processQuitEvent()
{
   setRunning(false);
}

//--------------------------------------------------------------------------

void DefaultGame::processTimeEvent(TimeEvent *event)
{
    PROFILE_START(ProcessTimeEvent);
   U32 elapsedTime = event->elapsedTime;

   if(elapsedTime > 1024)
   {
      elapsedTime = 0;
   }

   if(gTimeAdvance)
   {
      elapsedTime = gTimeAdvance;
   }
   else if ( mNotEqual( gTimeScale, 1.0f ) )
   {
      elapsedTime = (U32) (elapsedTime * gTimeScale);
   }
   {
       elapsedTime = elapsedTime;
   }

   Platform::advanceTime(elapsedTime);
   bool tickPass;

    PROFILE_START(ServerProcess);
#ifdef TORQUE_OS_IOS_PROFILE
iPhoneProfilerStart("SERVER_PROC");
#endif    
    tickPass = gServerProcessList.advanceTime(elapsedTime);
#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerEnd("SERVER_PROC");
#endif
    PROFILE_END();	

   PROFILE_START(ServerNetProcess);
   // only send packets if a tick happened
   if(tickPass)
      GNet->processServer();
   PROFILE_END();
    
   PROFILE_START(SimAdvanceTime);
#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerStart("SIM_TIME");
#endif
    Sim::advanceTime(elapsedTime);
#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerEnd("SIM_TIME");
#endif
    PROFILE_END();

   PROFILE_START(ClientProcess);
#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerStart("CLIENT_PROC");
#endif

   PROFILE_START(TickableAdvanceTime);
   Tickable::advanceTime(elapsedTime);	
   PROFILE_END();

    // This is based on PW's stuff
#ifndef NO_AUDIO_SUPPORT

    // Milliseconds between audio updates.
    const U32 AudioUpdatePeriod = 125;

   // alxUpdate is somewhat expensive and does not need to be updated constantly,
   // though it does need to be updated in real time
   static U32 lastAudioUpdate = 0;
   U32 realTime = Platform::getRealMilliseconds();
   if((realTime - lastAudioUpdate) >= AudioUpdatePeriod)
   {
      alxUpdate();
      lastAudioUpdate = realTime;
   }
#endif

#ifdef TORQUE_OS_IOS_PROFILE
    iPhoneProfilerEnd("CLIENT_PROC");
#endif
    PROFILE_END();
   PROFILE_START(ClientNetProcess);
      GNet->processClient();
   PROFILE_END();
    
    Process::processEvents();
    
   GNet->checkTimeouts();
    
#ifdef TORQUE_ALLOW_MUSICPLAYER
    updateVolume();
#endif
   PROFILE_END();
}


//--------------------------------------------------------------------------

void DefaultGame::processConsoleEvent(ConsoleEvent *event)
{
    char *argv[2];
    argv[0] = (char*)"eval";
    argv[1] = event->data;
    Sim::postCurrentEvent(Sim::getRootGroup(), new SimConsoleEvent(2, const_cast<const char**>(argv), false));
}

//--------------------------------------------------------------------------

void DefaultGame::processPacketReceiveEvent(PacketReceiveEvent * prEvent)
{
   GNet->processPacketReceiveEvent(prEvent);
}

