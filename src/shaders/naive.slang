import "./include/uv.slang";
import "./include/raymarching.slang";

struct Params {
    float2 resolution;
    uint rayCount;
    uint maxSteps;
}

ParameterBlock<Params> params;

layout(binding = 0) Sampler2D tex;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
   return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
    float4 light = tex.Sample(in.uv);

    if (light.a > 0.1) return light;

    float oneOverRayCount = 1.0 / float(params.rayCount);
    float tauOverRayCount = TAU * oneOverRayCount;

    let noise = rand(in.uv);

    float4 radiance = {0.0, 0.0, 0.0, 1.0};

    for (uint i = 0; i < params.rayCount; i++) {
        float angle = tauOverRayCount * (float(i) + noise);
        float2 rayDirUv = float2(cos(angle), -sin(angle)) / params.resolution;

        float2 sampleUv = in.uv + rayDirUv;

        for (uint step = 0; step < params.maxSteps; ++step) {
            if (outOfUv(sampleUv)) break;

            float4 sample = tex.Sample(sampleUv);
            if (sample.a > 0.1) {
                radiance += sample;
                break;
            }

            sampleUv += rayDirUv;
        }
    }

    return radiance * oneOverRayCount;
}
