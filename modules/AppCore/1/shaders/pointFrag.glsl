uniform sampler2D sampler2d_0;

in vec4 DestinationColor;
out vec4 FragColor;

void main() 
{  
    vec4 texColor_0 = texture(sampler2d_0, gl_PointCoord) * DestinationColor;
    FragColor = texColor_0;
}
