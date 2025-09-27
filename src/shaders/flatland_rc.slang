import "./include/uv.slang";
import "./include/raymarching.slang";

struct Constants {
    float2 resolution;
    uint baseRayCount;
    uint maxSteps;
    uint cascadeCount;
}

struct Params {
    uint currentCascade;
}

layout(binding = 0) ConstantBuffer<Constants> constants;
layout(binding = 1) ConstantBuffer<Params> params;

layout(binding = 0) Sampler2D sceneTex;
layout(binding = 1) Sampler2D distanceTex;
layout(binding = 2) Sampler2D lastTex;

static const float srgb = 2.2;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
   return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
    float sqrtBaseRayCount = sqrt(float(constants.baseRayCount));
    float shortestSide = min(constants.resolution.x, constants.resolution.y);
    float2 scale = shortestSide / constants.resolution;

    float cascadeIndex = float(params.currentCascade);

    float intervalStart = cascadeIndex == 0.0 ? 0.0 : pow(constants.baseRayCount, cascadeIndex - 1.0) / shortestSide;
    float intervalLength = pow(constants.baseRayCount, cascadeIndex) / shortestSide;

    float rayCount = pow(constants.baseRayCount, cascadeIndex + 1.0);
    float spacing = pow(sqrtBaseRayCount, cascadeIndex);

    float2 coord = floor(in.uv * constants.resolution);
    float4 radiance = float4(0.0);

    bool isFirstLevel = rayCount == constants.baseRayCount;

    float oneOverRayCount = 1.0 / rayCount;
    float angleStepSize = TAU * oneOverRayCount;

    // Number of probes in each dimension
    float2 size = floor(constants.resolution / spacing);

    // Which probe are we in?
    float2 probeRelativePosition = coord % size;

    // Which group of rays are we in?
    float2 rayPos = floor(coord / size);


    // Center of this probe
    float2 probeCenter = (probeRelativePosition + 0.5) * spacing;
    float2 normalizedProbeCenter = probeCenter / constants.resolution;

    float baseIndex = float(constants.baseRayCount) * (rayPos.x + (spacing * rayPos.y));

    float2 oneOverRes = 1.0 / constants.resolution;

    float minStepSize = min(oneOverRes.x, oneOverRes.y) * 0.5;

    // Shoot rays in "rayCount" directions, equally spaced.
    for (int i = 0; i < constants.baseRayCount; i++) {
        float index = baseIndex + float(i);
        float angleStep = index + 0.5;
        float angle = angleStepSize * angleStep;
        float2 rayDirection = float2(cos(angle), -sin(angle));

        float2 sampleUv = normalizedProbeCenter + intervalStart * rayDirection * scale;
        float4 radDelta = float4(0.0);
        float traveled = intervalStart;

        // We tested uv already (we know we aren't an object), so skip step 0.
        for (int step = 1; step < constants.maxSteps; step++) {
            float dist = distanceTex.Sample(sampleUv).r;

            // Go the direction we're traveling
            sampleUv += rayDirection * dist * scale;

            if (outOfUv(sampleUv)) break;

            if (dist <= minStepSize) {
                float4 colorSample = sceneTex.Sample(sampleUv);
                radDelta += colorSample;
                break;
            }

            traveled += dist;
            if (traveled >= intervalLength) break;
        }

        // Only merge on non-opaque areas
        if (params.currentCascade < constants.cascadeCount - 1 && radDelta.a == 0.0) {
          float upperSpacing = pow(sqrtBaseRayCount, cascadeIndex + 1.0);
          // Grid of probes
          float2 upperSize = floor(constants.resolution / upperSpacing);
          // Position of this probe
          float2 upperPosition = float2(index % upperSpacing, floor(index / upperSpacing)) * upperSize;

          float2 offset = (probeRelativePosition + 0.5) / sqrtBaseRayCount;
          float2 clamped = clamp(offset, float2(0.5), upperSize - 0.5);
          float2 upperUv = (upperPosition + clamped) / constants.resolution;

          float4 upperSample = lastTex.Sample(upperUv);

          radDelta += upperSample;
        }

        // Accumulate total radiance
        radiance += radDelta;
    }

    float3 final = (radiance.rgb / float(constants.baseRayCount));

    return float4(params.currentCascade != 0 ? final : pow(final, float3(1.0 / srgb)), 1.0);
}
