#ifdef GL_ES

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

#else

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

#endif