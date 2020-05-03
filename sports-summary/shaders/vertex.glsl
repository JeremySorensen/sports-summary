#version 330 core
layout (location = 0) in vec2 xy;
layout (location = 1) in vec2 st;
layout (location = 2) in float texture;

out vec2 TexCoord;
out float Texture;

void main()
{
    gl_Position = vec4(xy, 0.0, 1.0);
    TexCoord = st;
    Texture = 1.0 - texture;
}