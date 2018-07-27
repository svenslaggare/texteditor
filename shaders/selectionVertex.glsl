#version 330 core

uniform mat4 projection;

in vec2 vertexPosition;
in vec3 vertexColor;

out vec3 color;

void main()
{
    color = vertexColor;
    gl_Position = projection * vec4(vertexPosition, 0.0, 1.0);
}