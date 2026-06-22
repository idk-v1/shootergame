#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 colorMul;

uniform sampler2D tex;

void main()
{
	FragColor = texture(tex, texCoord) * vec4(colorMul, 1.f);
}
