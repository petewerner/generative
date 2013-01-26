#version 120
#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect u_denstex;
uniform float spread;
uniform sampler2DRect u_randtex;
uniform float u_randres;
uniform vec2 u_densmax;


#define TWO_PI 6.283185307

//debugging main for v6
void main(void)
{
    vec2 rloc = gl_TexCoord[0].st;// * 1.75;
    vec2 rvals = texture2DRect(u_randtex, rloc).xy;
    if (rvals.x == 0.0 && rvals.y == 0.0) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {

        gl_FragColor = vec4(rvals.x, rvals.y, 0.0, 1.0);
    }
//    vec2 rvals = texture2DRect(u_denstex, rloc).xy;
//    gl_FragColor = vec4(vec3(rvals.x), 1.0);
}

//this was pre the v6 dja import
void a_main(void)
{
    vec2 rloc = gl_TexCoord[0].st;
    vec2 rvals = texture2DRect(u_randtex, rloc).xy;
    gl_FragColor = vec4(rvals.x, rvals.y, 0.0, 1.0);
}


//this is old pre v5 cruft that can be deleted
void old_main(void)
{
    //denstex is a 0.0 - 1.0 mapping of the density of the attractor.
    //here we generate a velocity map for each x, y across the attractor
    //this will be used to update the particles
    vec2 dens_loc = gl_TexCoord[0].st; 
    vec4 dens_val = texture2DRect(u_denstex, dens_loc);

    float speed = 1.0; //this too
    float angle = dens_val.x * TWO_PI * spread;
    float cval = cos(angle);
    float sval = sin(angle);
    float x_dir = speed * cval;
    float y_dir = speed * sval;

    //add random jitter
    vec2 rand_loc = dens_loc / 700;
    rand_loc *= u_randres;
    vec2 rv = texture2DRect(u_randtex, rand_loc).xy;

    float dens_scale = 1.0;//spread;
    x_dir += rv.x;// * dens_val.x * dens_scale;
    y_dir += rv.y;// * dens_val.x * dens_scale;

    gl_FragColor = vec4(x_dir,y_dir, 0.0, 1.0);
    
}

//c is the density map value
//float angle = c * TWO_PI * spread;
//float cang = cos(angle) ;
//float sang = sin(angle) ;
//
//float speed = 1.0f;
//vel[i][0] = speed * cang;
//vel[i][1] = speed * sang;
//vel[i][0] += ofRandom(-1.0, 1.0);
//vel[i][1] += ofRandom(-0.5, 0.5);

