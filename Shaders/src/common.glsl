struct CameraData {
    vec4 position;
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
};

struct PushConstants {
    mat4 modelMat;
    uint objectID;
    uint textureID;
};

struct Light {
    vec4 position;
    vec4 rotation;
    vec4 color;
    uvec4 flags;
};

struct LightData {
    uint count;
};