#ifdef GL_ES
precision lowp float;

uniform sampler2D sampler2d_0;

varying vec4 vertColor;
varying vec2 TexCoord;

void main()
{  
    vec4 texColor_0 = texture2D(sampler2d_0, TexCoord) * vertColor;
    gl_FragColor = texColor_0;
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
