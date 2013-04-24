#ifdef GL_ES
precision lowp float;

uniform mat4 mvp_matrix; 
attribute vec4 Position;
attribute vec4 SourceColor;
varying vec4 DestinationColor;

void main()
{  
    DestinationColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;

    gl_PointSize = Position.w;
}

#else

uniform mat4 mvp_matrix; 
in vec4 Position;
in vec4 SourceColor;   
out vec4 DestinationColor;

void main()
{  
    DestinationColor = SourceColor;
    vec4 test = vec4( Position.xyz, 1.0);
    gl_Position = mvp_matrix * test;

    gl_PointSize = Position.w;
}

#endif
