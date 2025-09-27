import "./include/uv.slang";
import "./include/raymarching.slang";

struct Params {
    float2 resolution;
    uint rayCount;
    uint baseRayCount;
    uint maxSteps;
}

ParameterBlock<Params> params;

layout(binding = 0) Sampler2D sceneTex;
layout(binding = 1) Sampler2D lookupTex;
layout(binding = 2) Sampler2D lastTex;

static const float EPS = 0.001f;
static const float srgb = 2.2;
static const float partial = 0.125;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
   return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
    float2 uv = in.uv;

    float2 cascadeExtent = params.resolution;
    float2 coord = floor(uv * cascadeExtent);

    float4 radiance = float4(0.0);

    float oneOverRayCount = 1.0 / float(params.rayCount);
    float angleStepSize = TAU * oneOverRayCount;

    bool isLastLayer = params.rayCount == params.baseRayCount;

    float intervalStart = isLastLayer ? 0.0 : partial;
    float intervalEnd = isLastLayer ? partial : sqrt(2.0);

    float2 effectiveUv = isLastLayer ? uv : (floor(coord / 2.0) * 2.0) / params.resolution;

    float2 scale = min(params.resolution.x, params.resolution.y) / params.resolution;
    float2 oneOverSize = 1.0 / params.resolution;
    float minStepSize = min(oneOverSize.x, oneOverSize.y) * 0.5;

    // Shoot rays in "rayCount" directions, equally spaced.
    for (int i = 0; i < params.rayCount; i++) {
        float index = float(i);
        float angleStep = (index + 0.5);
        float angle = angleStepSize * angleStep;
        float2 rayDirection = float2(cos(angle), -sin(angle));

        float2 sampleUv = effectiveUv + rayDirection * intervalStart * scale;
        float4 radDelta = float4(0.0);

        float traveled = intervalStart;

        // We tested uv already (we know we aren't an object), so skip step 0.
        for (int step = 1; step < params.maxSteps; step++) {
            float2 nearestPoint = lookupTex.Sample(sampleUv).xy;
            float dist = length(nearestPoint - sampleUv);

            // Go the direction we're traveling
            sampleUv += rayDirection * dist * scale;

            if (outOfUv(sampleUv)) break;

            if (dist < minStepSize) {
                float4 colorSample = sceneTex.Sample(sampleUv);
                radDelta += float4(pow(colorSample.rgb, float3(srgb)), 1.0);
                break;
            }

            traveled += dist;
            if (traveled >= intervalEnd) break;
        }

        // Only merge on non-opaque areas
        if (isLastLayer && radDelta.a == 0.0) {
          float4 upperSample = lastTex.Sample(in.uv);

          radDelta += float4(pow(upperSample.rgb, float3(srgb)), upperSample.a);
        }

        // Accumulate total radiance
        radiance += radDelta;
    }

    float3 final = (radiance.rgb * oneOverRayCount);

    return float4(pow(final, float3(1.0 / srgb)), 1.0);
}
