#ifdef GL_ES
precision lowp float;

uniform sampler2D sampler2d_0;

varying vec4 vertColor;
varying vec2 TexCoord;

void main()
{  
    vec4 texColor_0 = texture2D(sampler2d_0, TexCoord);
    gl_FragColor.a = texColor_0.r * vertColor.a;
    gl_FragColor.rgb = vertColor.rgb;
}

#else

uniform sampler2D sampler2d_0;

in vec4 vertColor;
in vec2 TexCoord;
out vec4 FragColor;

void main()
{  
    vec4 texColor_0 = texture(sampler2d_0, TexCoord);
    FragColor.a = texColor_0.r;
    FragColor.rgb = vertColor.rgb;
}

#endif
