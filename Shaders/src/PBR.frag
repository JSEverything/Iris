#version 450 core
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (set = 1, binding = 0) uniform sampler2D texAlbedo[];
layout (set = 1, binding = 1) uniform sampler2D texNormal[];
layout (set = 1, binding = 2) uniform sampler2D texMetallic[];
layout (set = 1, binding = 3) uniform sampler2D texHeight[];
layout (set = 1, binding = 4) uniform sampler2D texRoughness[];
layout (set = 1, binding = 5) uniform sampler2D texAO[];

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstants {
    layout (offset = 64) uint textureID;
};

void main()
{
    ivec2 size = textureSize(texAlbedo[textureID], 0);
    if (size.x + size.y <= 2) {
        outColor = vec4(0, 0.5, 0, 1); // empty texture
        return;
    }

    outColor = vec4(texture(texAlbedo[textureID], inUV).rgb, 1.0);
}