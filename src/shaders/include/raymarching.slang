static const float PI = 3.14159265;
static const float TAU = 2.0 * PI;

float rand(float2 co) {
    return fract(sin(dot(co.xy, float2(12.9898,78.233))) * 43758.5453);
}