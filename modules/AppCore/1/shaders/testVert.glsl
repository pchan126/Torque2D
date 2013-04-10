#ifdef GL_ES
precision lowp float;
#endif

uniform mat4 mvp_matrix; 
in vec4 Position;
in vec4 SourceColor;
in vec3 Normal;
out vec4 DestinationColor;  

void main() 
{  
    DestinationColor = SourceColor;
    gl_Position = mvp_matrix * Position;
}