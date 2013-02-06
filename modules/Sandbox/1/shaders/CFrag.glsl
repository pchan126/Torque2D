uniform vec4 blend_0;
in vec4 DestinationColor; 
out vec4  FragColor;

void main()
{  
    FragColor.xyzw = DestinationColor.xyzw;
}