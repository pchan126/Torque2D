#ifdef GL_ES
precision lowp float;

uniform sampler2D sampler2d_0;

uniform vec4 vertColor;
uniform vec2 TexCoord;

void main()
{  
    vec4 texColor_0 = texture2D(sampler2d_0, TexCoord) * vertColor;
    gl_FragColor = texColor_0;
     gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0);
}

#else

uniform sampler2D sampler2d_0;

in vec4 vertColor;
in vec2 TexCoord;
out vec4 FragColor;

void main()
{  
    vec4 texColor_0 = texture(sampler2d_0, TexCoord) * vertColor;
    FragColor = texColor_0;
}

#endif
