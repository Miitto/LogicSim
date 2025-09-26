import "./include/uv.slang";

struct Params {
  float2 offset;
};

ParameterBlock<Params> params;

layout(binding = 0) Sampler2D inTex;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
  return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
  float2 nearestSeed = float2(-2.0);
  float nearestDist = 999999.9;
  
  for (float y = -1.0; y <= 1.0; y += 1.0) {
    for (float x = -1.0; x <= 1.0; x += 1.0) {
      float2 sampleUV = in.uv + float2(x, y) * params.offset;
      
      // Check if the sample is within bounds
      if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0) { continue; }
      
        float4 sampleValue = inTex.Sample(sampleUV);
        float2 sampleSeed = sampleValue.xy;
        
        if (sampleSeed.x != 0.0 || sampleSeed.y != 0.0) {
          float2 diff = sampleSeed - in.uv;
          float dist = dot(diff, diff);
          if (dist < nearestDist) {
            nearestDist = dist;
            nearestSeed = sampleValue.xy;
          }
        }
    }
  }
  return float4(nearestSeed, 0.0, 1.0);
}
