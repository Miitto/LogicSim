import "./include/uv.slang";

layout(binding = 0) Sampler2D tex;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
  return basicVertex(in);
}

[shader("fragment")]
float4 frag(BasicVOut in) : SV_Target {
  let alpha = tex.Sample(in.uv).a;
  return float4(in.uv * alpha, 0.0, 1.0);
}
