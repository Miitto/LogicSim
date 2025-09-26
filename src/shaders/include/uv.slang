float2 uvFromPosition(float2 pos) {
  var uv = pos;
  uv = (uv + 1) / 2;
  return uv;
}

bool outOfUv(float2 pos) {
    return pos.x < 0. || pos.x > 1. || pos.y < 0. || pos.y > 1.;
}

struct BasicVIn {
    float2 position : POSITION;
};

struct BasicVOut {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

BasicVOut basicVertex(BasicVIn i) {
    BasicVOut o;
    o.position = float4(i.position, 0, 1);
    o.uv = uvFromPosition(i.position);
    return o;
}