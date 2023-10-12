layout(set=0, binding=0) uniform UniformCamera
{
	mat4  projection;
	mat4  view;
    mat4  invProjection;
    mat4  invView;
    mat4  stereoProjection[2];
    mat4  stereoView[2];
    mat4  invStereoProjection[2];
    mat4  invStereoView[2];
    vec4  zBufferParams; // x = 1-far/near, y = far/near,  z = x/far, w = y/far
    vec4  pixelSize; // camera's pixelWidth, pixelHeight, 1.0 / pixelWidth, 1.0f / pixelHeight
	vec4  cameraPosition;
} scene;

float Linear01Depth(float z)
{
    return 1.0 / (scene.zBufferParams.x * z + scene.zBufferParams.y);
}

float LinearEyeDepth(float z)
{
    return 1.0 / (scene.zBufferParams.z * z + scene.zBufferParams.w);
}