//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "./gfxGLESOcclusionQuery.h"
#import <OpenGLES/ES2/glext.h>

// GL_EXT_occlusion_query_boolean, iPad2+ & iOS 5.0

GFXGLESOcclusionQuery::GFXGLESOcclusionQuery(GFXDevice* device) : 
   GFXOcclusionQuery(device), mQuery(0)
{
   glGenQueriesEXT(1, &mQuery);
}

GFXGLESOcclusionQuery::~GFXGLESOcclusionQuery()
{
   glDeleteQueriesEXT(1, &mQuery);
}

bool GFXGLESOcclusionQuery::begin()
{
   glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, mQuery);
   return true;
}

void GFXGLESOcclusionQuery::end()
{
   glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
}

GFXOcclusionQuery::OcclusionQueryStatus GFXGLESOcclusionQuery::getStatus(bool block, U32* data)
{
   // If this ever shows up near the top of a profile 
   // then your system is GPU bound.
   PROFILE_SCOPE(GFXGLOcclusionQuery_getStatus);
   
   GLuint numPixels = 0;
   GLuint queryDone = false;
   
   if (block)
      queryDone = true;
   else
      glGetQueryObjectuivEXT(mQuery, GL_QUERY_RESULT_AVAILABLE_EXT, &queryDone);
   
   if (queryDone)
      glGetQueryObjectuivEXT(mQuery, GL_QUERY_RESULT_EXT, &numPixels);
   else
      return Waiting;
   
   if (data)
      *data = numPixels;
   
   return NotOccluded;
}

void GFXGLESOcclusionQuery::zombify()
{
   glDeleteQueriesEXT(1, &mQuery);
   mQuery = 0;
}

void GFXGLESOcclusionQuery::resurrect()
{
   glGenQueriesEXT(1, &mQuery);
}

const String GFXGLESOcclusionQuery::describeSelf() const
{
   // We've got nothing
   return String();
}
