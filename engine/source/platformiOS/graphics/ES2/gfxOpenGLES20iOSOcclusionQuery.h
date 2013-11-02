//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFX_GL_OCCLUSIONQUERY_H_
#define _GFX_GL_OCCLUSIONQUERY_H_

#ifndef _GFXOCCLUSIONQUERY_H_
#include "gfx/gfxOcclusionQuery.h"
#endif
#import <OpenGLES/ES2/glext.h>

class GFXGLESOcclusionQuery : public GFXOcclusionQuery
{
public:
   GFXGLESOcclusionQuery( GFXDevice *device );
   virtual ~GFXGLESOcclusionQuery();

   virtual bool begin();
   virtual void end();
   virtual OcclusionQueryStatus getStatus( bool block, U32 *data = NULL );

   // GFXResource
   virtual void zombify(); 
   virtual void resurrect();
   virtual const String describeSelf() const;
   
private:
   GLuint mQuery;
};

#endif // _GFX_GL_OCCLUSIONQUERY_H_
