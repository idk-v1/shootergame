#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 colorMul;

uniform sampler2D tex;

//uniform int lod;

void main()
{
	//switch (lod)
	//{
	//case 0: FragColor = vec4(1.f, 0.f, 0.f, 1.f); break;
	//case 1: FragColor = vec4(1.f, 1.f, 0.f, 1.f); break;
	//case 2: FragColor = vec4(0.f, 1.f, 0.f, 1.f); break;
	//case 3: FragColor = vec4(0.f, 1.f, 1.f, 1.f); break;
	//}
	FragColor = texture(tex, texCoord) * vec4(colorMul, 1.f);
}