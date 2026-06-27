#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertColor;
out vec2 texCoord;

uniform float yMin; uniform float yMax;
uniform float zMin; uniform float zMax;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertColor = aColor;
    texCoord = vec2((aPos.y - yMin)/(yMax - yMin), (aPos.z - zMin)/(zMax - zMin));
}
