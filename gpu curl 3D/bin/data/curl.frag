#version 330

out vec4 FragColor;
uniform sampler2D uPtex;
uniform vec2 dims;
uniform float gradScale;

vec2
Grad(float x, float y)
{
    float eps = 1.0/dims.x;
    float h = 1/dims.x * 0.5;
    float n1, n2, dx, dy;
    vec2 dx0, dx1, dy0, dy1;

    dx0 = vec2(x, y - eps) + h;
    dx1 = vec2(x, y + eps) + h;
    dy0 = vec2(x - eps, y) + h;
    dy1 = vec2(x + eps, y) + h;
    
    n1 = texture(uPtex, dx0).r;
    n2 = texture(uPtex, dx1).r;
    dx = n1 - n2;

    n1 = texture(uPtex, dy0).r;
    n2 = texture(uPtex, dy1).r;
    dy = n1 - n2;
    vec2 g = vec2(dx, dy) / (2 * eps);
    g *= gradScale;

    return g;
}

void
main()
{
    vec2 loc = gl_FragCoord.xy / dims;

    vec2 g = Grad(loc.x, loc.y);
    FragColor = vec4(g.x, 0.0, g.y, 1.0);
    
}