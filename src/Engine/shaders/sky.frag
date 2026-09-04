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
    if (TexCoords.y < -0.1f)
        discard;

    const vec4 midnight = vec4(0.000f, 0.000f, 0.125f, 1.f);
    const vec4 night    = vec4(0.125f, 0.000f, 0.188f, 1.f);
    const vec4 morning  = vec4(0.169f, 0.333f, 0.500f, 1.f);
    const vec4 day      = vec4(0.408f, 0.733f, 0.824f, 1.f);
    const vec4 midday   = vec4(0.408f, 0.733f, 0.824f, 1.f);

    const vec4 sunset   = vec4(1.000f, 0.500f, 0.000f, 1.f);
    const vec4 sun      = vec4(1.000f, 1.000f, 0.900f, 1.f);


    vec3 sunPos = vec3(0.f, sin(time), cos(time));
    float skystate = sunPos.y;
    vec4 color = vec4(0.f, 0.f, 0.f, 0.f);
    if (skystate >= 0.5f)
        color = mix(day, midday, (skystate - 0.5f) * 2.f);
    else if (skystate >= 0.0f)
        color = mix(morning, day, (skystate - 0.0f) * 2.f);
    else if (skystate >= -0.5f)
        color = mix(night, morning, (skystate + 0.5f) * 2.f);
    else if (skystate >= -1.0f)
        color = mix(midnight, night, (skystate + 1.0f) * 2.f);

    float horizonSunDist = length((sunPos - TexCoords) * vec3(1.f, 3.f, 1.f));
    float sunsetStrength = clamp((0.3f - abs(sunPos.y)) * 3.33f, 0.f, 1.f);
    color = mix(color, mix(color, sunset, 1.f / horizonSunDist), sunsetStrength);
    
    float sunRad = 0.05f;
    float sunDist = length(sunPos - TexCoords);

    // why is the power needed??? vvv
    color = mix(sun, color, pow(clamp(sunDist - sunRad, 0.f, 1.f), 0.5f));

	FragColor = color;
}