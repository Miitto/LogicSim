import "./include/uv.slang";
import "./include/raymarching.slang";

struct Params {
    float2 resolution;
    uint rayCount;
    uint maxSteps;
}

ParameterBlock<Params> params;

layout(binding = 0) Sampler2D sceneTex;
layout(binding = 1) Sampler2D lookupTex;

static const float EPS = 0.001f;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
   return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
    float4 light = sceneTex.Sample(in.uv);

    if (light.a > 0.1) return light;

    float oneOverRayCount = 1.0 / float(params.rayCount);
    float tauOverRayCount = TAU * oneOverRayCount;

    let noise = rand(in.uv);

    float4 radiance = {0.0, 0.0, 0.0, 0.0};

    for (uint i = 0; i < params.rayCount; i++) {
        float angle = tauOverRayCount * (float(i) + noise);
        float2 rayDir = float2(cos(angle), -sin(angle));

        float2 sampleUv = in.uv;
        float4 radDelta = float4(0.0);

        // If not, we went out of bounds
        bool hitSurface = false;

        for (uint step = 1; step < params.maxSteps; ++step) {
            float2 nearestPoint = lookupTex.Sample(sampleUv).xy;
            float dist = length(nearestPoint - sampleUv);

            sampleUv += rayDir * dist;

            if (outOfUv(sampleUv)) break;

            if (dist < EPS) {
              float4 sample = sceneTex.Sample(sampleUv);
              radDelta += sample;
              hitSurface = true;
              break;
            }
        }

        radiance += radDelta;
    }

    return float4(max(light, radiance * oneOverRayCount).rgb, 1.0);
}
