struct vIn {
    float3 pos;
    float3 color;
};

struct v2f {
    float4 pos : SV_POSITION;
    float3 color;
};

[shader("vertex")]
v2f vert(vIn v) {
  v2f o;
  o.pos = float4(v.pos, 1.0);
  o.color = v.color;
  return o;
}

[shader("fragment")]
float4 frag(v2f i) : SV_Target {
  float4 color = float4(i.color, 1.0);

  color.rgb = pow(color.rgb, 1.0 / 2.2); // Gamma correction

  return color;
}