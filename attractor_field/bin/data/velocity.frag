#version 120

uniform sampler2DRect partloc;
uniform sampler2DRect curltex;
uniform sampler2DRect veltex;

void
main(void)
{
    vec2 st = gl_TexCoord[0].st;
    vec2 ploc = texture2DRect(partloc, st).xy;
    vec2 curl = texture2DRect(curltex, ploc).rg; // get curl val of current part loc
    vec2 vel = texture2DRect(veltex, st).xy;
    
//    curl *= 2;
    
    if (abs(vel.x + curl.x) < 0.01 && abs(vel.y + curl.y) < 0.01) {
        
        // see if chg to if (abs(curl.x) > 0.1) vel.x = curl.x oth keep vel.xy as is
        if (abs(curl.x) < 0.10)
            vel.x = 1.0;
        else
            vel.x = curl.x;
        
        if (abs(curl.y) < 0.10)
            vel.y = 1.0;
        else
            vel.y = curl.y;
    }
    
    if (abs(curl.x) > 0.2)
        vel.x = curl.x;
    if (abs(curl.y) > 0.2)
        vel.y = curl.y;
    
    gl_FragColor = vec4(vel.x, vel.y, 0.0, 1.0);
//    gl_FragColor = vec4(vel.y, 0.0, 0.0, 1.0);
//    gl_FragColor = vec4(1.0);
}


//if (abs(it->z + cur_curl.x) < 0.01 && abs(it->w + cur_curl.y) < 0.01) {
//    if (abs(cur_curl.x) < 0.10)
//        it->z = 1.0;
//    else
//        it->z = cur_curl.x;
//    if (abs(cur_curl.y) < 0.10)
//        it->w = 1.0;
//    else
//        it->w = cur_curl.y;
//}