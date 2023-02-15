#version 450 core
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (set = 1, binding = 0) uniform sampler2D textures[];

layout (location = 0) out vec4 outColor;
layout (location = 1) out uint outID;

layout (push_constant) uniform PushConstants {
    mat4 modelMat;
    uint objectID;
    bool isBillboard;
};

void main()
{
    outID = objectID + 1;

    ivec2 size = textureSize(textures[objectID], 0);
    if (size.x + size.y <= 2) {
        outColor = vec4(0, 0.5, 0, 1); // empty texture
        return;
    }

    outColor = vec4(texture(textures[objectID], inUV).rgb, 1.0);
}