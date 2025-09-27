import "./include/uv.slang";

layout(binding = 0) Sampler2D tex;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
  return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
    float2 nearestSeed = tex.Sample(in.uv).xy;
    float dist = clamp(distance(in.uv, nearestSeed), 0.0, 1.0);

    return float4(dist, dist, dist, 1.0);
}
