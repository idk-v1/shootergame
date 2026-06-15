#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec2 texCoord;
out vec3 colorMul;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 modelMat;

uniform vec3 camPos;

uniform mat4x3 lights[150];
uniform int lightStates[10]; // ceil(300 / 32) = 10
uniform int lightCount;

struct Light
{
	vec3  rgb;
	vec3  pos;
	vec3  norm;
	float str;
	int type;
};

Light lightFromMat(mat4x3 mat)
{
	Light light;
	light.rgb  = mat[0];
	light.pos  = mat[1];
	light.norm = mat[2];
	light.str  = mat[3][0];
	light.type = int(mat[3][1]);
	return light;
}

void main()
{
	gl_Position = projMat * viewMat * modelMat * vec4(pos, 1.0);
	texCoord = tex;

	vec3 modelNorm = normalize(mat3(transpose(inverse(modelMat))) * norm);
	vec3 vertPos = (modelMat * vec4(pos, 1.0)).xyz;

	vec3 colorMulTemp = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < lightCount; ++i)
	{
		if ((lightStates[i >> 5] & (1 << (i & 31))) != 0)
		{
			Light light = lightFromMat(lights[i]);

			switch (light.type)
			{
			case 0: // Disabled
				{
					break;
				}
			case 1: // Ambient Light
				{
					colorMulTemp += light.rgb;
					break;
				}

			case 2: // Directional Light
				{
					float normDiff = max(dot(modelNorm, normalize(light.norm)), 0.0);
					colorMulTemp += light.rgb * normDiff;
					break;
				}

			case 3: // Point Light
				{
					float normDiff = max(dot(modelNorm, normalize(light.pos - vertPos)), 0.0);

					float dist = length(light.pos - vertPos);

					float strength = max(light.str - dist, 0.0) / light.str;
					colorMulTemp += light.rgb * normDiff * strength;
					break;
				}

			case 4: // Directional point light
				{
					float normDiffDir = max(dot(modelNorm, normalize(light.norm)), 0.0);
					float normDiffPt = max(dot(modelNorm, normalize(light.pos - vertPos)), 0.0);

					float lookDir = max(dot(normalize(light.pos - vertPos), normalize(light.norm)), 0.0);

					float dist = length(light.pos - vertPos);
					float strength = max(light.str - dist, 0.0) / light.str;
					colorMulTemp += light.rgb * normDiffDir * normDiffPt * lookDir * strength;
					break;
				}
			}

		}

	}

	colorMul = colorMulTemp;
}