#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    vec4 color = texture(skybox, TexCoords);
    if (color.w > 0)
        FragColor = color;
    else
        discard;
}