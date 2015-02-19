#version 330

out vec4 FragColor;
uniform sampler2D uPtex;
uniform vec2 dims;

vec2
Grad(float x, float y)
{
    float eps = 1.0/dims.x;
    float h = 1.0/dims.x * 0.5;
    
    float n1, n2, dx, dy;
    vec2 dx0, dx1, dy0, dy1;
    dx0 = vec2(x - eps, y);
    dx1 = vec2(x + eps, y);
    dy0 = vec2(x, y - eps);
    dy1 = vec2(x, y + eps);
    
    n1 = texture(uPtex, dx0 + h).r;
    n2 = texture(uPtex, dx1 + h).r;
    dx = n1 - n2;
    
    n1 = texture(uPtex, dy0 + h).r;
    n2 = texture(uPtex, dy1 + h).r;
    dy = n1 - n2;

    vec2 g = vec2(dx, dy) * 10.0f;
    return g;
}


vec2
Curl(float x, float y)
{
    vec2 g = Grad(x, y);
    return vec2(g.y, -g.x);
}

void
main()
{
    vec2 loc = gl_FragCoord.xy;
    loc /= dims;

    vec2 g = Curl(loc.s, loc.t);
    //i use the blue channel just cause I like blue more than green
    FragColor = vec4(g.x, 0.0, g.y, 1.0);
    
}