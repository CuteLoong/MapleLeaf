#ifndef STOCHASITIC_SSR_COMMON_GLSL
#define STOCHASITIC_SSR_COMMON_GLSL

void generateBasis(vec3 N, out vec3 up, out vec3 right, out vec3 forward)
{
    up = abs(N.z) < 0.999f ? vec3(0, 0, 1) : vec3(1, 0, 0);
    right = normalize(cross(up, N));
    forward = cross(N, right);
}

vec3 localToWorld(vec3 localVector, vec3 N)
{
	vec3 up, right, forward;
	generateBasis(N, up, right, forward);

	return normalize(right * localVector.x + forward * localVector.y + N * localVector.z);
}

float evalGGX(float ggxAlpha, float NdotH)
{
    float a2 = ggxAlpha * ggxAlpha;
    float d = ((NdotH * a2 - NdotH) * NdotH + 1.0f);
    return a2 / (M_PI * d * d);
}

// UE4 GGX-Smith Correlated Joint Approximate
// Note: V = G / (4 * NdotL * NdotV)
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJointApprox(float NdotL, float NdotV, float roughness)
{
    float a = roughness * roughness;
    float Vis_SmithV = NdotL * (NdotV * (1 - a) + a);
    float Vis_SmithL = NdotV * (NdotL * (1 - a) + a);
    return 0.5 / (Vis_SmithV + Vis_SmithL);
}

vec3 ImportanceSampleGGX(vec2 u, vec3 N, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float phi = M_2PI * u.x;
    float cosTheta = sqrt((1 - u.y) / (1 + (a2 - 1) * u.y));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);

    // Tangent space H
    vec3 tH;
    tH.x = sinTheta * cos(phi);
    tH.y = sinTheta * sin(phi);
    tH.z = cosTheta;

    vec3 up, right, forward;
    generateBasis(N, up, right, forward);

    // World space H
    return normalize(right * tH.x + forward * tH.y + N * tH.z);
}

ivec2 GetHiZLevelSize(float mipLevel)
{
    return ivec2(max(1, camera.stereoPixelSize.x / pow(2.0f, mipLevel)), max(1, camera.stereoPixelSize.y / pow(2.0f, mipLevel)));
}

ivec2 GetHizPixel(vec2 pointXY, vec2 levelSize)
{
    return ivec2(floor((pointXY.x) * levelSize.x), floor(pointXY.y * levelSize.y));
}

vec2 GetZMinMaxFromHiZ(ivec2 pixel, float mipLevel, int levelSizeX, int viewIndex)
{
    vec2 zMinMax;
    zMinMax.x = viewIndex == 0 ? Linear01Depth(texelFetch(inMinHiZLeft, pixel, int(mipLevel)).x) :  Linear01Depth(texelFetch(inMinHiZRight, pixel, int(mipLevel)).x);
    zMinMax.y = viewIndex == 0 ? Linear01Depth(texelFetch(inMaxHiZLeft, pixel, int(mipLevel)).x) : Linear01Depth(texelFetch(inMaxHiZRight, pixel, int(mipLevel)).x);
    return zMinMax;
}

void minMaxHiZTraversalLoop(vec2 step, vec2 stepOffset, vec2 stepEye2, vec2 stepOffsetEye2, vec3 pointSS0, vec3 rayDir, vec2 pointSS0Eye2XY, vec2 rayDirEye2XY, float calcT0, float calcT1, inout int viewIndex, inout float mipLevel, inout float tParameter, inout vec2 tSceneZMinMax, inout int iterations)
{
    // float iterations = 0.0f;
    int curViewIndex;
    while(mipLevel >= 0.0f && iterations < ssrData.maxSteps && tParameter <= 1.0f) {
        iterations++;
        curViewIndex = viewIndex;

        vec2 maxRayPointXY = pointSS0.xy + rayDir.xy * tParameter;
        vec2 maxRayPointXY2 = pointSS0Eye2XY + rayDirEye2XY * tParameter;

        bool outOfBounds1 = (maxRayPointXY.x < 0.0f) || (maxRayPointXY.x > 1.0f) || (maxRayPointXY.y > 1.0f || maxRayPointXY.y < 0.0f);
        bool outOfBounds2 = (maxRayPointXY2.x < 0.0f) || (maxRayPointXY2.x > 1.0f) || (maxRayPointXY2.y > 1.0f || maxRayPointXY2.y < 0.0f);

        if(outOfBounds1 && outOfBounds2) {
            tParameter = 1.0f;
            break;
        }

        ivec2 levelSize = GetHiZLevelSize(mipLevel);
        ivec2 pixel = GetHizPixel(maxRayPointXY, levelSize);

        vec2 tPixelXY = ((pixel + step) / levelSize + stepOffset - pointSS0.xy) / rayDir.xy;
        float tPixel = min(tPixelXY.x, tPixelXY.y);

        vec2 sceneZMinMax = GetZMinMaxFromHiZ(pixel, mipLevel, levelSize.x, viewIndex);
        tSceneZMinMax = (1.0f / sceneZMinMax + vec2(calcT0)) * calcT1;

        // use another eyes info
        // if(tSceneZMinMax.y <= tParameter || (outOfBounds1 && !outOfBounds2)) {
        //     curViewIndex = 1 - curViewIndex;
        //     maxRayPointXY = pointSS0Eye2XY + rayDirEye2XY * tParameter;
        //     pixel = GetHizPixel(maxRayPointXY, levelSize);

        //     tPixelXY = ((pixel + stepEye2) / levelSize + stepOffsetEye2 - pointSS0Eye2XY) / rayDirEye2XY;
        //     tPixel = min(tPixelXY.x, tPixelXY.y);
        //     sceneZMinMax = GetZMinMaxFromHiZ(pixel, mipLevel, levelSize.x, curViewIndex);
        //     tSceneZMinMax = (1.0f / sceneZMinMax + vec2(calcT0)) * calcT1;
        // }

        mipLevel--;
        if(tSceneZMinMax.x <= tPixel && tParameter <= tSceneZMinMax.y) {
            tParameter = max(tParameter, tSceneZMinMax.x);
        }
        else {
            tParameter = tPixel;
            mipLevel = min(ssrData.hiZMaxLevel, mipLevel + 2.0f);
        }
    }
    viewIndex = curViewIndex;
}

void maxMinHiZTraversalLoop(vec2 step, vec2 stepOffset, vec2 stepEye2, vec2 stepOffsetEye2, vec3 pointSS0, vec3 rayDir, vec2 pointSS0Eye2XY, vec2 rayDirEye2XY, float calcT0, float calcT1, inout int viewIndex, inout float mipLevel, inout float tParameter, inout vec2 tSceneZMinMax, inout int iterations)
{
    // float iterations = 0.0f;
    int curViewIndex;
    while(mipLevel >= 0.0f && iterations < ssrData.maxSteps && tParameter <= 1.0f) {
        iterations++;
        curViewIndex = viewIndex;

        vec2 maxRayPointXY = pointSS0.xy + rayDir.xy * tParameter;
        vec2 maxRayPointXY2 = pointSS0Eye2XY + rayDirEye2XY * tParameter;

        bool outOfBounds1 = (maxRayPointXY.x < 0.0f) || (maxRayPointXY.x > 1.0f) || (maxRayPointXY.y > 1.0f || maxRayPointXY.y < 0.0f);
        bool outOfBounds2 = (maxRayPointXY2.x < 0.0f) || (maxRayPointXY2.x > 1.0f) || (maxRayPointXY2.y > 1.0f || maxRayPointXY2.y < 0.0f);

        if(outOfBounds1 && outOfBounds2) {
            tParameter = 1.0f;
            break;
        }

        ivec2 levelSize = GetHiZLevelSize(mipLevel);
        ivec2 pixel = GetHizPixel(maxRayPointXY, levelSize);

        vec2 tPixelXY = ((pixel + step) / levelSize + stepOffset - pointSS0.xy) / rayDir.xy;
        float tPixel = min(tPixelXY.x, tPixelXY.y);

        vec2 sceneZMinMax = GetZMinMaxFromHiZ(pixel, mipLevel, levelSize.x, viewIndex);
        tSceneZMinMax = (1.0f / sceneZMinMax + vec2(calcT0)) * calcT1;

        // use another eyes info
        // if(outOfBounds1 && !outOfBounds2) {
        //     curViewIndex = 1 - curViewIndex;
        //     maxRayPointXY = pointSS0Eye2XY + rayDirEye2XY * tParameter;
        //     pixel = GetHizPixel(maxRayPointXY, levelSize);

        //     tPixelXY = ((pixel + stepEye2) / levelSize + stepOffsetEye2 - pointSS0Eye2XY) / rayDirEye2XY;
        //     tPixel = min(tPixelXY.x, tPixelXY.y);
        //     sceneZMinMax = GetZMinMaxFromHiZ(pixel, mipLevel, levelSize.x, curViewIndex);
        //     tSceneZMinMax = (1.0f / sceneZMinMax + vec2(calcT0)) * calcT1;
        // }

        mipLevel--;
        if(tSceneZMinMax.y <= tPixel && tParameter <= tSceneZMinMax.x) {
            tParameter = max(tParameter, tSceneZMinMax.y);
        }
        else {
            tParameter = tPixel;
            mipLevel = min(ssrData.hiZMaxLevel, mipLevel + 2.0f);
        }
    }
    viewIndex = curViewIndex;
}


#endif //STOCHASITIC_SSR_COMMON_GLSL