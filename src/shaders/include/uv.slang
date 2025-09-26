float2 uvFromPosition(float2 pos) {
  var uv = pos;
  uv.y *= -1;
  uv = (uv + 1) / 2;
  return uv;
}