uniform sampler2D sampler2d_0;

in vec4 vertColor;
in vec2 TexCoord;
out vec4 FragColor;

void main()
{  
    vec4 texColor_0 = texture(sampler2d_0, TexCoord);
    FragColor.a = texColor_0.r * vertColor.a;
    FragColor.rgb = vertColor.rgb;
}