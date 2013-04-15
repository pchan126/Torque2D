#ifdef GL_ES
precision lowp float;

uniform vec4 blend_0;
uniform vec4 DestinationColor;

void main()
{  
    gl_FragColor.xyzw = vec4(1.0, 1.0, 0.0, 1.0);
}

#else

uniform vec4 blend_0;
in vec4 DestinationColor; 
out vec4  FragColor;

void main()
{  
    FragColor.xyzw = vec4(1.0, 1.0, 0.0, 1.0);
}

#endif
