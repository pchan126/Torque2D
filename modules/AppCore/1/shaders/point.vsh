#ifdef GL_ES
precision lowp float;

uniform mat4 mvp_matrix; 
attribute vec4 Position;
attribute vec4 SourceColor;
attribute float Size;
varying vec4 DestinationColor;

void main()
{  
    DestinationColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;

    gl_PointSize = Size;
}

#else

uniform mat4 mvp_matrix; 
in vec4 Position;
in vec4 SourceColor;
in float Size;
out vec4 DestinationColor;

void main()
{  
    DestinationColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;

    gl_PointSize = Size;
}

#endif
