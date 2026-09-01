#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform float time;

// https://github.com/shff/opengl_sky

float hash(float n)
{
    return fract(sin(n) * 43758.5453123);
}

float noise(vec3 x)
{
  vec3 f = fract(x);
  float n = dot(floor(x), vec3(1.0, 157.0, 113.0));
  return mix(mix(mix(hash(n +   0.0), hash(n +   1.0), f.x),
                 mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
             mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                 mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}

const mat3 m = mat3(0.0, 1.60,  1.20, -1.6, 0.72, -0.96, -1.2, -0.96, 1.28);
float fbm(vec3 p)
{
    float f = 0.0;
    f += noise(p) / 2; p = m * p * 1.1;
    f += noise(p) / 4; p = m * p * 1.2;
    f += noise(p) / 6; p = m * p * 1.3;
    f += noise(p) / 12; p = m * p * 1.4;
    f += noise(p) / 24;
    return f;
}


void main()
{
	const vec4 noClouds     = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	const vec4 thinClouds   = vec4(0.8f, 0.8f, 0.8f, 0.6f);
	const vec4 normalClouds = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	const vec4 darkClouds   = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	const float scale = 5.f;
	vec3 coords = vec3(TexCoords.x, TexCoords.y + time * 0.002f, TexCoords.z);
	float cloud = pow(fbm(coords * scale), 1.2f);

	if (cloud < 0.25f)
		discard;                                                                // [0.00 - 0.25]
	else if (cloud < 0.50f)
		FragColor = mix(noClouds, thinClouds, pow((cloud - 0.25f) * 4.f, 3.f)); // [0.25 - 0.50]
	else if (cloud < 0.75f)
		FragColor = mix(thinClouds, normalClouds, (cloud - 0.50f) * 4.f);       // [0.50 - 0.75]
	else
		FragColor = mix(normalClouds, darkClouds, (cloud - 0.75f) * 4.f);       // [0.75 - 1.00]
}