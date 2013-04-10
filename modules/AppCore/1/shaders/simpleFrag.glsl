#ifdef GL_ES
precision lowp float;
#endif

uniform sampler2D sampler2d_0;

in vec4 vertColor;
in vec2 TexCoord;
out vec4 FragColor;

void main()
{  
    vec4 texColor_0 = texture(sampler2d_0, TexCoord) * vertColor;
    FragColor = texColor_0;
    //     FragColor = vec4(1.0, 0.5, 0.0, 1.0);
}