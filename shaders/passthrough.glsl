#version 150 core

uniform sampler2D inputTexture;

in vec2 textureCoord;
out vec4 outputColor;

void main()
{
    outputColor = texture(inputTexture, textureCoord);
}