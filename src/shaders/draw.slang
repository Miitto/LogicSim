import "./include/uv.slang";

struct Draw {
    float2 from;
    float2 to;
    float4 color;
    float2 resolution;
};

ParameterBlock<Draw> draw;

struct vIn {
    float2 position : POSITION;
};

struct vOut {
    float4 position : SV_Position;
    float2 uv;
}

[shader("vertex")]
vOut vert(vIn in) {
  vOut output;
  output.position = float4(in.position, 0.0, 1.0);
  output.uv = uvFromPosition(in.position);
  output.uv *= draw.resolution;
  return output;
}

float sdfLineSquared(float2 pos, float2 from, float2 to) {
    let toStart = pos - from;
    let line = to - from;
    let lineLengthSq = dot(line, line);
    let t = clamp(dot(toStart, line) / lineLengthSq, 0.0, 1.0);
    let closest = toStart - line * t;
    return dot(closest, closest);
}

[shader("fragment")]
float4 frag(vOut input) : SV_Target {
  float2 coord = input.uv;

  if (sdfLineSquared(coord, draw.from, draw.to) > (draw.color.w * draw.color.w)) {
      discard;
  }

  return float4(draw.color.xyz, 1.0);
}