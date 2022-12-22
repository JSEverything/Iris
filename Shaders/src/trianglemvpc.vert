#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec4 inNormal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

layout (binding = 0) uniform MVPC {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 clip;
} mvpc;

void main ()
{
    gl_Position = mvpc.clip * mvpc.projection * mvpc.view * mvpc.model * inPosition;
    outColor = vec3(inColor);
    outNormal = vec3(inNormal);
}