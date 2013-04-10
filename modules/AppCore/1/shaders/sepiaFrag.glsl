#ifdef GL_ES
precision lowp float;
#endif

uniform sampler2D sampler2d_0; 
varying vec4 DestinationColor; 
varying vec2 TexCoord; 

void main() 
{  
    vec4 color = texture2D(sampler2d_0, TexCoord) * DestinationColor; 
    gl_FragColor.r = (color.r * 0.393) + (color.g * 0.769) + (color.b * 0.189); 
    gl_FragColor.g = (color.r * 0.349) + (color.g * 0.686) + (color.b * 0.168);    
    gl_FragColor.b = (color.r * 0.272) + (color.g * 0.534) + (color.b * 0.131);
    gl_FragColor.a = color.a; 
}