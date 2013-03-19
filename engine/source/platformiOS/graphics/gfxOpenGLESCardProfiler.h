//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLCARDPROFILE_H
#define _GFXGLCARDPROFILE_H

#include "graphics/gfxCardProfile.h"

class GFXOpenGLESCardProfiler : public GFXCardProfiler
{
public:
   void init();

protected:
   virtual const String& getRendererString() const { return mRendererString; }
   virtual void setupCardCapabilities();
   virtual bool _queryCardCap(const String& query, U32& foundResult);
   virtual bool _queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips);

private:
   String mRendererString;
   typedef GFXCardProfiler Parent;
};

#endif
