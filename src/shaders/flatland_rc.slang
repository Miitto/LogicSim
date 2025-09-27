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

static const float srgb = 2.2;
static const float partial = 0.125;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
   return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
    float2 coord = floor(in.uv * params.resolution);
    float4 radiance = float4(0.0);

    bool isLastLevel = params.rayCount == 256;
    bool isFirstLevel = params.rayCount == params.baseRayCount;

    float sqrtBaseRayCount = sqrt(float(params.baseRayCount));
    float oneOverRayCount = 1.0 / float(params.rayCount);
    float angleStepSize = TAU * oneOverRayCount;

    // Space between probes
    float spacing = isFirstLevel ? 1.0 : sqrtBaseRayCount;

    // Number of probes in each dimension
    float2 size = floor(params.resolution / spacing);

    // Which probe are we in?
    float2 probeRelativePosition = coord % size;

    // Which group of rays are we in?
    float2 rayPos = floor(coord / size);

    float intervalStart = isFirstLevel ? 0.0 : partial;
    float intervalEnd = isFirstLevel ? partial : sqrt(2.0);

    // Center of this probe
    float2 probeCenter = (probeRelativePosition + 0.5) * spacing;
    float2 normalizedProbeCenter = probeCenter / params.resolution;

    float baseIndex = float(params.baseRayCount) * (rayPos.x + (spacing * rayPos.y));

    float2 oneOverRes = 1.0 / params.resolution;
    float2 scale = min(params.resolution.x, params.resolution.y) * oneOverRes;

    float minStepSize = min(oneOverRes.x, oneOverRes.y) * 0.5;

    // Shoot rays in "rayCount" directions, equally spaced.
    for (int i = 0; i < params.baseRayCount; i++) {
        float index = baseIndex + float(i);
        float angleStep = index + 0.5;
        float angle = angleStepSize * angleStep;
        float2 rayDirection = float2(cos(angle), -sin(angle));

        float2 sampleUv = normalizedProbeCenter + intervalStart * rayDirection * scale;
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
                radDelta += float4(pow(colorSample.rgb, float3(srgb)), colorSample.a);
                break;
            }

            traveled += dist;
            if (traveled >= intervalEnd) break;
        }

        // Only merge on non-opaque areas
        if (isFirstLevel && radDelta.a == 0.0) {
          // sqrtBaseRayCount is the spacing between probes
          // Grid of probes
          float2 upperSize = floor(params.resolution / sqrtBaseRayCount);
          // Position of this probe
          float2 upperPosition = float2(index % sqrtBaseRayCount, floor(index / sqrtBaseRayCount)) * upperSize;

          float2 offset = (probeRelativePosition + 0.5) / sqrtBaseRayCount;
          float2 upperUv = (upperPosition + offset) / params.resolution;

          float4 upperSample = lastTex.Sample(upperUv);

          radDelta += upperSample;
        }

        // Accumulate total radiance
        radiance += radDelta;
    }

    float3 final = (radiance.rgb * oneOverRayCount);

    return float4(!isLastLevel ? final : pow(final, float3(1.0 / srgb)), 1.0);
}
