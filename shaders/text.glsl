#version 330 core

uniform sampler2D inputTexture;

in vec2 textureCoord;
in vec3 color;
out vec4 outputColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(inputTexture, textureCoord).r);
    outputColor = vec4(color, 1.0) * sampled;
}

