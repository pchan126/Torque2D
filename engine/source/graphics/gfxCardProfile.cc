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
#include "graphics/gfxCardProfile.h"

#include "platform/platform.h"
#include "console/console.h"
#include "io/resource/resourceManager.h"



GFXCardProfiler::GFXCardProfiler()
{
}

GFXCardProfiler::~GFXCardProfiler()
{
   mCapDictionary.clear();
}

void GFXCardProfiler::init()
{
   // Spew a bit...
   Con::printf("Initializing GFXCardProfiler (%s)", getRendererString().c_str());
   Con::printf("   o Chipset : '%s'", getChipString().c_str());
   Con::printf("   o Card    : '%s'", getCardString().c_str());
   Con::printf("   o Version : '%s'", getVersionString().c_str());

   // Do card-specific setup...
   Con::printf("   - Scanning card capabilities...");

   setupCardCapabilities();
}

U32 GFXCardProfiler::queryProfile(const String &cap)
{
   U32 res;
   if( _queryCardCap( cap, res ) )
      return res;

   if(mCapDictionary.contains(cap))
      return mCapDictionary[cap];

   Con::errorf( "GFXCardProfiler (%s) - Unknown capability '%s'.", getRendererString().c_str(), cap.c_str() );
   return 0;
}

U32 GFXCardProfiler::queryProfile(const String &cap, U32 defaultValue)
{
   PROFILE_SCOPE( GFXCardProfiler_queryProfile );

   U32 res;
   if( _queryCardCap( cap, res ) )
      return res;

   if( mCapDictionary.contains( cap ) )
      return mCapDictionary[cap];
   else
      return defaultValue;
}

void GFXCardProfiler::setCapability(const String &cap, U32 value)
{
   // Check for dups.
   if( mCapDictionary.contains( cap ) )
   {
      Con::warnf( "GFXCardProfiler (%s) - Setting capability '%s' multiple times.", getRendererString().c_str(), cap.c_str() );
      mCapDictionary[cap] = value;
      return;
   }

   // Insert value as necessary.
   Con::printf( "GFXCardProfiler (%s) - Setting capability '%s' to %d.", getRendererString().c_str(), cap.c_str(), value );
   mCapDictionary.insert( cap, value );
}

bool GFXCardProfiler::checkFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips )
{
   return _queryFormat( fmt, profile, inOutAutogenMips );
}


ConsoleStaticMethod( GFXCardProfilerAPI, getVersion, const char *, 2, 2,
   "Returns the driver version string." )
{
	return GFX->getCardProfiler()->getVersionString();
}

ConsoleStaticMethod( GFXCardProfilerAPI, getCard, const char *, 2, 2,
   "Returns the card name." )
{
	return GFX->getCardProfiler()->getCardString();
}

ConsoleStaticMethod( GFXCardProfilerAPI, getVendor, const char *, 2, 2,
   "Returns the card vendor name." )
{
   // TODO: Fix all of this vendor crap, it's not consistent
	return GFX->getCardProfiler()->getChipString();
}

ConsoleStaticMethod( GFXCardProfilerAPI, getRenderer, const char *, 2, 2,
   "Returns the renderer name.  For example D3D9 or OpenGL." )
{
	return GFX->getCardProfiler()->getRendererString();
}

ConsoleStaticMethod( GFXCardProfilerAPI, setCapability, void, 4, 4,
   "Used to set the value for a specific card capability.\n"
   "@param name The name of the capability being set.\n"
   "@param value The value to set for that capability." )
{
    const char *name = argv[2];
    S32 value = dAtoi( argv[3] );
	GFX->getCardProfiler()->setCapability( name, (U32)value );
}

ConsoleStaticMethod( GFXCardProfilerAPI, queryProfile, S32, 4, 4,
   "Used to query the value of a specific card capability.\n"
   "@param name The name of the capability being queried.\n"
   "@param defaultValue The value to return if the capability is not defined." )
{
    const char *name = argv[2];
    S32 defaultValue = dAtoi( argv[3] );
	return (S32)GFX->getCardProfiler()->queryProfile( name, (U32)defaultValue );
}
