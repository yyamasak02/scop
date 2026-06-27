#version 330 core
in vec3 vertColor;
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D tex;
uniform float blendFactor;

void main()
{
    vec4 color = vec4(vertColor, 1.0);
    vec4 texColor = texture(tex, texCoord);
    FragColor = mix(color, texColor, blendFactor);
}
