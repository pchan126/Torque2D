#include "graphics/OpenGL/gfxOpenGLCardProfiler.h"


void GFXOpenGLCardProfiler::init()
{
   mChipSet = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

   // get the major and minor parts of the GL version. These are defined to be
   // in the order "[major].[minor] [other]|[major].[minor].[release] [other] in the spec
   const char *versionStart = reinterpret_cast<const char*>(glGetString(GL_VERSION));
   const char *versionEnd = versionStart;
   // get the text for the version "x.x.xxxx "
   for( S32 tok = 0; tok < 2; ++tok )
   {
      char *text = dStrdup( versionEnd );
      dStrtok(text, ". ");
      versionEnd += dStrlen( text ) + 1;
      dFree( text );
   }

   mRendererString = "GL";
   mRendererString += String::SpanToString(versionStart, versionEnd - 1);

   mCardDescription = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
   mVersionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));

   Parent::init();
}

void GFXOpenGLCardProfiler::setupCardCapabilities()
{
   GLint maxTexSize;
   GLint numCompressedTexFormats;
   
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
   
   const char* versionString = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
   
   Con::printf("OpenGl Shading Language Version: %s", versionString);
   
   // OpenGL doesn't have separate maximum width/height.
   setCapability("maxTextureWidth", maxTexSize);
   setCapability("maxTextureHeight", maxTexSize);
   setCapability("maxTextureSize", maxTexSize);
   
   glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTexFormats);
   setCapability("numCompressedTextureFormats", numCompressedTexFormats);
}