#version 120

//color the particles based on the dja texture
//alpha values come via the vertex shader

uniform sampler2DRect partloc;
uniform sampler2DRect djatex;
uniform sampler2DRect colortex;
varying float alpha;
uniform vec2 screensize;
uniform vec2 texsize;
varying vec2 ploc;

void
a_main(void)
{
    vec4 color = texture2DRect(colortex, gl_FragColor.xy);
    gl_FragColor = color;
}

void
a_main(void)
{
    vec2 loc = gl_FragCoord.xy;
    loc /= screensize;
    loc *= texsize;
    vec3 incol = texture2DRect(djatex, loc).rgb;
//    gl_FragColor = vec4(vec3(incol.rgb), alpha);
    gl_FragColor = vec4(alpha);
//    gl_FragColor = vec4(incol, 1.0);
}
