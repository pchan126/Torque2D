#ifdef GL_ES
precision lowp float;
#endif

uniform mat4 mvp_matrix;
uniform vec4 Position;
uniform vec4 SourceColor;
uniform vec3 Normal;
varying vec4 DestinationColor;  

void main() 
{  
    DestinationColor = SourceColor;
    gl_Position = mvp_matrix * Position;
}