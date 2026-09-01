#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 modelMat;

uniform float radius;

void main()
{
    gl_Position = projMat * viewMat * modelMat * vec4(aPos, 1.f);
    TexCoords = aPos * radius / 200.f;
}
