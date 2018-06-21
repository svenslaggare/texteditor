#version 330 core

in vec2 vertexPosition;
in vec2 vertexTexcoord;

out vec2 textureCoord;

void main()
{
    textureCoord = vertexTexcoord;
    gl_Position = vec4(vertexPosition, 0.0, 1.0);
}