#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;
in float Texture;

uniform sampler2D texture_0;
uniform sampler2D texture_1;

void main()
{
    FragColor = mix(texture(texture_0, TexCoord), texture(texture_1, TexCoord), Texture);
}