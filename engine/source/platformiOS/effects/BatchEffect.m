//
//  BatchEffect.m
//  Torque2D
//
//  Created by Paul L Jan on 2013-06-21.
//

#import "BatchEffect.h"

@implementation BatchEffect

- (id)init
{
   if (!(self = [super init])) return nil;

   // Save the current context, just in case
   EAGLContext* currCtx = [EAGLContext currentContext];
   if (currCtx == nil ) return nil;
   if (currCtx.API != kEAGLRenderingAPIOpenGLES2) return nil;
   
   if(self != NULL)
   {
      
      self.colorMaterialEnabled = GL_FALSE;
      self.lightModelTwoSided = GL_FALSE;
      self.useConstantColor = GL_TRUE;
      
      self.lights = nil;
      
      self.lightingType = GLKLightingTypePerVertex;
      self.lightModelAmbientColor = GLKVector4Make(0.2, 0.2, 0.2, 1.0);

      self.material.ambientColor = GLKVector4Make(0.2, 0.2, 0.2, 1.0);           // { 0.2, 0.2, 0.2, 1.0}
      self.material.diffuseColor = GLKVector4Make(0.8, 0.8, 0.8, 1.0);           // { 0.8, 0.8, 0.8, 1.0}
      self.material.specularColor = GLKVector4Make(0.0, 0.0, 0.0, 1.0);          // { 0.0, 0.0, 0.0, 1.0}
      self.material.emissiveColor = GLKVector4Make(0.0, 0.0, 0.0, 1.0);          // { 0.0, 0.0, 0.0, 1.0}
      self.material.shininess = 0.0;              // 0.0

      self.texture2d0.enabled = false;
      self.texture2d1.enabled = false;

      self.textureOrder = [NSArray arrayWithObjects: self.texture2d0, self.texture2d1, nil];    // texture2d0, texture2d1
      self.constantColor = GLKVector4Make(1.0, 1.0, 1.0, 1.0);               // { 1.0, 1.0, 1.0, 1.0 }
      self.fog.enabled = false; // = GLKEffectPropertyFog alloc;                        // Disabled
      self.label = nil;
   }
   return self;
}


- (void) prepareToDraw
{
   
}

@end
