//
//  BatchEffect.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-06-21.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>

@interface BatchEffect : NSObject <GLKNamedEffect>
{
@protected
   
   // Switches to turn effect features on and off
   GLboolean                           _colorMaterialEnabled;
   GLboolean                           _fogEnabled;
   
   // Modelview, projection, texture and derived matrices for transformation
   GLKEffectPropertyTransform          *_transform;
   
   // Lights
   GLKLightingType                     _lightingType;
   NSArray                             *_lights;
   
   // Material for lighting
   GLKEffectPropertyMaterial           *_material;
   
   // GL Texture Names
   GLKEffectPropertyTexture            *_texture2d0, *_texture2d1;
   
   // Texture ordering array
   NSArray                             *_textureOrder;
   
   // Constant color (fixed color value to supplant the use of the "color" named vertex attrib array)
   GLKVector4                          _constantColor;
   
   // Fog
   GLKEffectPropertyFog                *_fog;
   
   // Label for effect
   NSString                            *_label;
   
   @private
}

- (void) prepareToDraw;

// Properties                                                                                           // Default Value

@property (nonatomic, assign)          GLboolean                           colorMaterialEnabled;        // GL_FALSE
@property (nonatomic, assign)          GLboolean                           lightModelTwoSided;          // GL_FALSE
@property (nonatomic, assign)          GLboolean                           useConstantColor;            // GL_TRUE

@property (nonatomic, readonly)        GLKEffectPropertyTransform          *transform;                  // Identity Matrices
@property (nonatomic, retain)        NSArray                             *lights;                     
@property (nonatomic, assign)          GLKLightingType                     lightingType;                // GLKLightingTypePerVertex
@property (nonatomic, assign)          GLKVector4                          lightModelAmbientColor;      // { 0.2, 0.2, 0.2, 1.0 }
@property (nonatomic, readonly)        GLKEffectPropertyMaterial           *material;                   // Default material state
@property (nonatomic, readonly)        GLKEffectPropertyTexture            *texture2d0, *texture2d1;    // Disabled
@property (nonatomic, retain)          NSArray                             *textureOrder;               // texture2d0, texture2d1
@property (nonatomic, assign)          GLKVector4                          constantColor;               // { 1.0, 1.0, 1.0, 1.0 }
@property (nonatomic, readonly)        GLKEffectPropertyFog                *fog;                        // Disabled

@property (nonatomic, retain)          NSString                            *label;                      // nil

@end
