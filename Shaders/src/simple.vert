#version 460 core
layout (location = 0) in vec3 inPos;

void main()
{
    gl_Position.xyz = inPos;
    gl_Position.w = 1.0;
}