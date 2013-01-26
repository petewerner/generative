#version 120
#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect tex1;
uniform sampler2DRect tex2;

void main(void)
{
    vec2 loc = gl_TexCoord[0].st;
//    vec2 vals = texture2DRect(tex1, loc).xy;
//    vec4 vals = texture2DRect(tex1, loc);
//    vals /= 700.0;
//    vec4 dens = texture2DRect(tex2, vals.xy);
//    float dval = texture2DRect(tex2, vals.xy).x;
//    gl_FragColor = dens;
//    gl_FragColor = vec4(dval);

    vec2 vel = texture2DRect(tex1, loc).xy;
    gl_FragColor = vec4(abs(vel.x));
}