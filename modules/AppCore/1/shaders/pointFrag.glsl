#ifdef GL_ES
precision lowp float;

uniform sampler2D sampler2d_0;

varying vec4 DestinationColor;

void main() 
{  
    vec4 texColor_0 = texture2D(sampler2d_0, gl_PointCoord) * DestinationColor;
    gl_FragColor = texColor_0;
}

#else

uniform sampler2D sampler2d_0;

in vec4 DestinationColor;
out vec4 FragColor;

void main() 
{  
    vec4 texColor_0 = texture(sampler2d_0, gl_PointCoord) * DestinationColor;
    FragColor = texColor_0;
}
#endif
