#version 450 core
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (set = 0, binding = 0) uniform CameraData1 {
    CameraData camera;
};

layout (set = 1, binding = 0) uniform sampler2D textures[];

layout (location = 0) out vec4 outColor;
layout (location = 1) out uint outID;

layout (push_constant) uniform PushConstants1 {
    PushConstants pc;
};

void main()
{
    outID = pc.objectID + 1;

    ivec2 size = textureSize(textures[pc.objectID], 0);
    if (size.x + size.y <= 2) {
        outColor = vec4(0.7f, 0.f, 0.7f, 1.f); // empty texture
        return;
    }
    outColor = texture(textures[pc.objectID], inUV).rgba;
    //if (outColor.a < 1.f) discard;
}