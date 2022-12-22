#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec4 inNormal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

layout (binding = 0) uniform model {
    mat4 model1;
};
layout (binding = 1) uniform view {
    mat4 view1;
};
layout (binding = 2) uniform projection {
    mat4 projection1;
};

void main ()
{
    gl_Position = projection1 * view1 * model1 * vec4(inPosition.xyz, 1.0f);
    outColor = vec3(inColor);
    outNormal = vec3(inNormal);
}