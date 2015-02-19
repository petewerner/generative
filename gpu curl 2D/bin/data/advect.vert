#version 330

in vec2 Position;
out vec2 vPosition;

uniform sampler2D Sampler;
uniform float Time;
const float UINT_MAX = 4294967295.0;

uniform vec2 dims;

uniform float curlamt;
uniform vec2 constVel;
uniform float advectDt;

uint randhash(uint seed)
{
    uint i=(seed^12345391u)*2654435769u;
    i^=(i<<6u)^(i>>26u);
    i*=2654435769u;
    i+=(i<<5u)^(i>>12u);
    return i;
}

float randhashf(uint seed, float b)
{
    return float(b * randhash(seed)) / UINT_MAX;
}

vec2 SampleVelocity2D(vec2 p)
{
    vec2 loc = p.xy;
    vec3 v = texture(Sampler, loc).rgb;
    return vec2(v.r, v.b);
}

void main()
{
    vPosition = Position;
    
    vec2 loc = Position.xy / dims;
    
    vec2 curlvel = SampleVelocity2D(loc.st);

    vPosition.xy += (constVel + curlvel * curlamt) * advectDt;
    
    if (Position.y > dims.y || Position.y < 1 || Position.x > dims.x || Position.x < 1) {
        uint seed = uint(Time * 1000.0) + uint(gl_VertexID);
        vPosition.x = randhashf(seed++, dims.x);
        vPosition.y = randhashf(seed++, dims.y);
    }

}
