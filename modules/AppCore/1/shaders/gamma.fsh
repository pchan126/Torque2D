#ifdef GL_ES
precision lowp float;

#define GammaCorrection(color, gamma)								pow(color, 1.0 / gamma)

uniform sampler2D sampler2d_0; 
uniform vec4 blend_0;
uniform vec3 gammaFactor;

varying vec4 DestinationColor; 
varying vec2 TexCoord; 

void main() 
{  
    vec4 texColor_0 = texture2D(sampler2d_0, TexCoord) * DestinationColor;
    texColor_0 = texColor_0 * blend_0;
    gl_FragColor.rgb = GammaCorrection(texColor_0.rgb, gammaFactor);
    gl_FragColor.a = texColor_0.a;
//    gl_FragColor = texColor_0;
    gl_FragColor.rgb = vec3(1.0, 0.0, 0.0);
} 

#else

#define GammaCorrection(color, gamma)								pow(color, 1.0 / gamma)

uniform sampler2D sampler2d_0; 
uniform vec4 blend_0;
uniform vec3 gammaFactor;

varying vec4 DestinationColor; 
varying vec2 TexCoord; 

void main() 
{  
    vec4 texColor_0 = texture2D(sampler2d_0, TexCoord) * DestinationColor;
    texColor_0 = texColor_0 * blend_0;
    gl_FragColor.rgb = GammaCorrection(texColor_0.rgb, gammaFactor);
    gl_FragColor.a = texColor_0.a;
//    gl_FragColor = texColor_0;
} 

#endif
