#version 330

in vec3 Position;
out vec3 vPosition;

uniform sampler2D uXY;
uniform sampler2D uXZ;
uniform sampler2D uYZ;

uniform float Time;
const float UINT_MAX = 4294967295.0;

uniform vec3 dims;

uniform float curlamt;
uniform vec3 constVel;
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


vec3 SampleVelocity(vec3 p)
{
    vec2 xy = texture(uXY, p.xy).rb;
    vec2 xz = texture(uXZ, p.xz).rb;
    vec2 yz = texture(uYZ, p.yz).rb;
    
    vec3 curl;
    curl.x = yz.y - yz.x;
    curl.y = xz.x - xz.y;
    curl.z = xy.y - xy.x;
    return curl;
}


void main()
{
    vPosition = Position;
    vec3 loc = vec3(Position.x, Position.y, abs(Position.z)) / dims;
    vec3 curlvel = SampleVelocity(loc.xyz);

    vPosition.xyz += (constVel + curlvel * curlamt) * advectDt;
    
    //place a particle randomly
    //in general you probably want a fixed emitter, say a 50x50x50 area
    //where particles come from. putting them randomly over a large area
    //tends to look a bit noisy/random funnily enough
    if (Position.y > dims.y || Position.y < 1 || Position.x > dims.x || Position.x < 1 || Position.z > 0 || Position.z < -dims.z) {
        uint seed = uint(Time * 1000.0) + uint(gl_VertexID);
        vPosition.x = randhashf(seed++, dims.x);
        vPosition.y = randhashf(seed++, dims.y);
        vPosition.z = -randhashf(seed++, dims.z);
    }

}
