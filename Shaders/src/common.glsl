struct CameraData {
    vec4 position;
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
};

struct PushConstants {
    mat4 modelMat;
    uint objectID;
};