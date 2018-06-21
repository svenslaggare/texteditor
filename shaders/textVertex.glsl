#version 330 core

uniform mat4 projection;

in vec2 vertexPosition;
in vec2 vertexTexcoord;
in vec3 vertexColor;

out vec2 textureCoord;
out vec3 color;

void main()
{
    textureCoord = vertexTexcoord;
    color = vertexColor;
    gl_Position = projection * vec4(vertexPosition, 0.0, 1.0);
}