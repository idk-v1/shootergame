#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out float relHeight;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 modelMat;

uniform float radius;
uniform float height;

void main()
{
    vec4 pos = modelMat * vec4(aPos, 1.f);
    gl_Position = projMat * viewMat * pos;
    relHeight = pos.y - height;
    TexCoords = aPos * radius / 200.f;
}
