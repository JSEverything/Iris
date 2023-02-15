#version 450 core
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec4 inNormal;
layout (location = 3) in vec2 inUV;

layout (set = 0, binding = 0) uniform ViewProjection {
    mat4 viewProjection;
};

layout (push_constant) uniform PushConstants {
    mat4 modelMat;
    uint objectID;
    bool isBillboard;
};

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;

void main() {
    vec4 locPos = modelMat * vec4(inPos.xyz, 1.0);

    outPosition = locPos.xyz / locPos.w;
    outNormal = inNormal.xyz;
    outUV = inUV;

    gl_Position = viewProjection * locPos;
}