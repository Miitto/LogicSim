import "./include/uv.slang";

struct vIn {
    float2 position : POSITION;
};

struct vOut {
    float4 position : SV_Position;
    float2 uv;
}

struct Params {
    float2 resolution;
}

ParameterBlock<Params> params;

[shader("vertex")]
vOut vert(vIn in) {
    vOut o;
    o.position = float4(in.position, 0.0, 1.0);
    o.uv = uvFromPosition(in.position);
    o.uv *= params.resolution;
    return o;
}

[shader("fragment")]
float4 frag(vOut in) : SV_Target {
    return float4(0.0, 1.0, 0.0, 1.0);
}
