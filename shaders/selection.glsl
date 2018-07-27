#version 330 core

in vec3 color;

out vec4 outputColor;

void main()
{
    outputColor = vec4(color, 0) + vec4(30 / 255.0f, 144 / 255.0f, 1, 0.5);
}