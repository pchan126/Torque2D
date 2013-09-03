#ifdef GL_ES
precision highp float;

uniform mat4 mvp_matrix; 
uniform mat4 modelview_matrix;

attribute vec4 Position;
attribute vec4 SourceColor;
attribute vec2 inTexCoord;

varying vec4 vertColor;
varying vec2 TexCoord;
varying vec3 ecPos;

void main()
{  
    /* compute the vertex position in camera space. */
    ecPos = modelview_matrix * Position.xyz;
    
    vertColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;
    TexCoord = inTexCoord;
}

#else

uniform mat4 mvp_matrix; 
uniform mat4 modelview_matrix;

in vec4 Position;
in  vec4 SourceColor;
in vec2 inTexCoord;

out vec4 vertColor;
out vec2 TexCoord;
out vec3 ecPos;

void main()
{  
    /* compute the vertex position in camera space. */
    ecPos = modelview_matrix * Position.xyz;

    vertColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;
    TexCoord = inTexCoord;
}

#endif
