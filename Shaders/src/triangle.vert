#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec4 normal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

vec2 positions[3] = vec2[](vec2 (0.0, -0.5), vec2 (0.5, 0.5), vec2 (-0.5, 0.5));

vec3 colors[3] = vec3[](vec3 (1.0, 0.0, 0.0), vec3 (0.0, 1.0, 0.0), vec3 (0.0, 0.0, 1.0));

void main ()
{
    gl_Position = position;
    outColor = vec3(color);
    outNormal = vec3(normal);
}