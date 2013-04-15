#ifdef GL_ES
precision lowp float;

uniform mat4 mvp_matrix; 

uniform vec4 Position;
uniform vec4 SourceColor;
uniform vec2 inTexCoord;

varying vec4 vertColor;
varying vec2 TexCoord;

void main()
{  
    vertColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;
    TexCoord = inTexCoord;
}

#else

uniform mat4 mvp_matrix; 

in vec4 Position;
in  vec4 SourceColor;
in vec2 inTexCoord;

out vec4 vertColor;
out vec2 TexCoord;

void main()
{  
    vertColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;
    TexCoord = inTexCoord;
}

#endif
