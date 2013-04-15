#ifdef GL_ES

precision mediump float;

uniform vec4 blend_0;
varying vec4 DestinationColor;

void main()
{  
     gl_FragColor = DestinationColor;
    gl_FragColor.rgb = vec3(1.0, 0.0, 0.0);
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

