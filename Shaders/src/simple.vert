#version 460 core
layout (location = 0) in vec4 inPos;

void main()
{
    gl_Position = inPos;
}