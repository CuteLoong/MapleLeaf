#ifndef MISC_CAMERA_GLSL
#define MISC_CAMERA_GLSL

layout(set=0, binding=0) uniform UniformCamera
{
	mat4 projection;
	mat4 view;
    mat4 invProjection;
    mat4 invView;
    mat4 stereoProjection[2];
    mat4 stereoView[2];
    mat4 invStereoProjection[2];
    mat4 invStereoView[2];
    vec4 frustumVector[4]; // leftTop, rightTop, leftBottom, rightBottom
    vec4 frustumPlane[6]; // left, right, bottom, top, near, far  normal(xyz)--d (ax+by+cz+d = 0)
    vec4 stereoLeftFrustumVector[4];
    vec4 stereoRightFrustumVector[4];
    vec4 stereoLeftFrustumPlane[6];
    vec4 stereoRightFrustumPlane[6];
    vec4 projectionParams; // x = 1 or -1 (-1 if projection is flipped), y = near plane, z = far plane, w = 1/far plane
    vec4 zBufferParams; // x = 1-far/near, y = far/near,  z = x/far, w = y/far
    vec4 pixelSize; // camera's pixelWidth, pixelHeight, 1.0 / pixelWidth, 1.0f / pixelHeight
    vec4 stereoPixelSize;
    vec4 cameraPosition;
    vec4 cameraStereoPosition[2];
    uint frameID;
} camera;

mat4 GetProjection(){ return camera.projection; }

mat4 GetView(){ return camera.view; }

vec4[6] GetFrustumPlanes(int viewIndex) {
    return viewIndex == 0 ? camera.stereoLeftFrustumPlane : camera.stereoRightFrustumPlane;
}

vec4[6] GetFrustumPlanes() {
    return camera.frustumPlane;
}

vec4 GetCameraPosition(int viewIndex) {
    return camera.cameraStereoPosition[viewIndex];
}

#endif
