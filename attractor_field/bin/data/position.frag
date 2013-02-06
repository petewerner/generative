#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect u_posloc;
uniform sampler2DRect u_randtex;
uniform sampler2DRect denstex;
uniform sampler2DRect veltex;
uniform vec2 u_screen;
uniform float u_part_life; //not used right now
uniform float speed;
uniform float widthOffset;

void main(void)
{
    vec2 st = gl_TexCoord[0].st;
    vec4 pos = texture2DRect(u_posloc, st);
    //each particle has fixed movement. randtex is the same size as part location map
    vec2 rand = texture2DRect(u_randtex, st).xy;
    vec2 vel = texture2DRect(veltex, st).xy;
    float dval = texture2DRect(denstex, pos.xy).x;
    
//    //the higher the density, the slower the move
//    vec2 velt = vel * (1.0 - dval);
//    //but make sure we dont stop
//    pos.x += max(0.2, abs(velt.x)) * sign(vel.x);
//    pos.y += max(0.2, abs(velt.y)) * sign(vel.y);
    vec2 velt = vel;
    pos.x += velt.x * speed;
    pos.y += velt.y * speed;

    if (dval > pos.w) { //update the alpha, used in particle vertex shader
//        pos.w = mix(pos.w, dval, 0.5);
//        pos.w = clamp(dval + dval * 0.5, dval, 1.0);
        pos.w = dval;
    }
    pos.w *= 0.96;  //uniform this eventually

    if (pos.w < 0.01 || pos.x < 0.0 || pos.x >= u_screen.x || pos.y < 0.0 || pos.y >= u_screen.y) {
    
//        vec2 rand = abs(vel);
        rand = abs(rand);
        //        pos.x = rand.x * u_screen.x;
        pos.y = rand.y * u_screen.y;
        pos.x = rand.x * u_screen.x;
        //        pos.y = u_screen.y - 1.0;
        //        pos.z = u_part_life;
        pos.w = 0.0;
        
    }
    gl_FragColor = vec4(pos.x, pos.y, pos.z, pos.w);
}
