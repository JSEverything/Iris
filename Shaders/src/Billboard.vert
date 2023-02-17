#version 450 core
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

layout (set = 0, binding = 0) uniform CameraData1 {
    CameraData camera;
};

layout (push_constant) uniform PushConstants1 {
    PushConstants pc;
};

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;

vec2 squarePositions[6] = {
{ -0.5f, -0.5f },
{ 0.5f, -0.5f },
{ -0.5f, 0.5f },
{ 0.5f, -0.5f },
{ 0.5f, 0.5f },
{ -0.5f, 0.5f }
};

void main() {
    vec3 cameraRight = { camera.view[0][0], camera.view[1][0], camera.view[2][0] };
    vec3 cameraUp = { camera.view[0][1], camera.view[1][1], camera.view[2][1] };

    vec3 scale = vec3(sqrt(pc.modelMat[0][0] * pc.modelMat[0][0] + pc.modelMat[0][1] * pc.modelMat[0][1] + pc.modelMat[0][2] * pc.modelMat[0][2]));

    vec3 position = vec3(pc.modelMat[3][0], pc.modelMat[3][1], pc.modelMat[3][2])
    + cameraRight * squarePositions[gl_VertexIndex].x * scale.x
    + cameraUp * squarePositions[gl_VertexIndex].y * scale.y;

    vec4 locPos = vec4(position, 1.f);
    outPosition = locPos.xyz / locPos.w;
    outNormal = vec3(- camera.view[0][2], - camera.view[1][2], - camera.view[2][2]);
    outUV = squarePositions[gl_VertexIndex].xy + 0.5f;

    gl_Position = camera.viewProjection * locPos;
}