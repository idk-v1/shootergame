#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 colorMul;

uniform sampler2D tex;

uniform float time;

void main()
{
	//vec2 off = vec2(cos(time + texCoord.x * 100.0), sin(time + texCoord.y * 100.0)) * 0.01;
	//if (off.x > 0.0025) off.x = 0.005 - off.x;
	//if (off.x <-0.0025) off.x = 0.005 + off.x;
	//if (off.y > 0.0025) off.y = 0.005 - off.y;
	//if (off.y <-0.0025) off.y = 0.005 + off.y;

	FragColor = texture(tex, texCoord) * vec4(colorMul, 1.0);
}