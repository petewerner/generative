#version 120

uniform sampler2DRect partloc;
uniform sampler2DRect colortex;
uniform sampler2DRect prevcol;
uniform vec2 screen;
uniform vec2 tex;

void
main(void)
{
    vec2 st = gl_TexCoord[0].st;
    //get the particle location
    vec4 part = texture2DRect(partloc, st);

    vec3 color;

    //color where the part is now
    vec3 new_color = texture2DRect(colortex, part.xy).xyz;
    //color where the part was last frame
    vec3 cur_color = texture2DRect(prevcol, st).xyz;
    if (new_color.r <= 0.05 && new_color.b <= 0.05) { //if its weak we just let the alpha fade out
        color = cur_color;
    } else {
        color = mix(cur_color, new_color, 0.2);
    }
    //part.w is controlled in position.frag
    gl_FragColor = vec4(color, part.w);
}