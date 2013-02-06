uniform sampler2DRect texmap;

void main(void)
{
    vec2 loc = gl_TexCoord[0].st;
    vec2 loc0 = vec2(loc.x, loc.y - 1.0);
    vec2 loc1 = vec2(loc.x - 1.0, loc.y);
    vec2 loc2 = vec2(loc.x + 1.0, loc.y);
    vec2 loc3 = vec2(loc.x, loc.y + 1.0);
    
    vec4 d = texture2DRect(texmap, loc);
    vec4 d0 = texture2DRect(texmap, loc0);
    vec4 d1 = texture2DRect(texmap, loc1);
    vec4 d2 = texture2DRect(texmap, loc2);
    vec4 d3 = texture2DRect(texmap, loc3);
    
    vec4 avg = d + d0 + d1 + d2 + d3;
    avg /= 5.0;
    
    gl_FragColor = vec4(avg);
    
}