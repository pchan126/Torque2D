#ifdef GL_ES
precision lowp float;

uniform vec4 blend_0;
in vec4 DestinationColor; 
out vec4  FragColor;

void main()
{  
     gl_FragColor.xyzw = DestinationColor.xyzw;
}
#else

uniform vec4 blend_0;
in vec4 DestinationColor; 
out vec4  FragColor;

void main()
{  
    FragColor.xyzw = DestinationColor.xyzw;
}
#endif

