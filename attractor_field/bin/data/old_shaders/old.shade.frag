uniform sampler2DRect texmap;

uniform float thresh;

void main()
{
    vec2 loc = gl_TexCoord[0].st;
    vec2 loc1 = vec2(loc.x-1.0, loc.y-1.0);
    vec2 loc2 = vec2(loc.x, loc.y-1.0);
    vec2 loc3 = vec2(loc.x+1.0, loc.y-1.0);
    vec2 loc4 = vec2(loc.x-1.0, loc.y);
    vec2 loc5 = vec2(loc.x, loc.y);
    vec2 loc6 = vec2(loc.x+1.0, loc.y);
    vec2 loc7 = vec2(loc.x-1.0, loc.y+1.0);
    vec2 loc8 = vec2(loc.x, loc.y+1.0);
    vec2 loc9 = vec2(loc.x+1.0, loc.y+1.0);
    
    float scale = 1.0/9.0;
    vec4 d1 = texture2DRect(texmap, loc1) * scale;
    vec4 d2 = texture2DRect(texmap, loc2) * scale;
    vec4 d3 = texture2DRect(texmap, loc3) * scale;
    vec4 d4 = texture2DRect(texmap, loc4) * scale;
    vec4 d5 = texture2DRect(texmap, loc5) * scale;
    vec4 d6 = texture2DRect(texmap, loc6) * scale;
    vec4 d7 = texture2DRect(texmap, loc7) * scale;
    vec4 d8 = texture2DRect(texmap, loc8) * scale;
    vec4 d9 = texture2DRect(texmap, loc9) * scale;

    vec4 fin = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
    if (fin.r < thresh)
        gl_FragColor = vec4(0.0);
    else
        gl_FragColor = vec4(vec3(fin.r), 1.0);

}

void inital_main(void)
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