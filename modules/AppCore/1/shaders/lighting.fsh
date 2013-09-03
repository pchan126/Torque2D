#ifdef GL_ES
precision lowp float;

uniform sampler2D sampler2d_0;
uniform int lightCount;
uniform vec3 lightPosition[10];
uniform vec3 lightColor[10];

varying vec4 vertColor;
varying vec2 TexCoord;
varying vec3 ecPos;

void main()
{  
    vec3 n, lightDir;
    
    lightDir = vec3(lightPosition[0] - ecPos);

    vec4 texColor_0 = texture2D(sampler2d_0, TexCoord) * vertColor;
    gl_FragColor = texColor_0;
}

#else

uniform sampler2D sampler2d_0;
uniform int lightCount;
uniform vec3 lightPosition[10];
uniform vec3 lightColor[10];

in vec4 vertColor;
in vec2 TexCoord;
in vec3 ecPos;
out vec4 FragColor;

void main()
{  
    vec4 texColor_0 = texture(sampler2d_0, TexCoord) * vertColor;
    FragColor = texColor_0;
}

#endif
