import "./include/uv.slang";

struct Draw {
    float2 from;
    float2 to;
    float4 color;
    float2 resolution;
};

ParameterBlock<Draw> draw;

[shader("vertex")]
BasicVOut vert(BasicVIn in) {
  var output = basicVertex(in);
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
float4 frag(BasicVOut input) : SV_Target {
  float2 coord = input.uv;

  var from = draw.from;
  from.y = draw.resolution.y - from.y;

  var to = draw.to;
  to.y = draw.resolution.y - to.y;

  if (sdfLineSquared(coord, from, to) > (draw.color.w * draw.color.w)) {
      discard;
  }

  return float4(draw.color.xyz, 1.0);
}