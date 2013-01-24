#version 120

uniform sampler2DRect tex;

void main()
{
    vec2 st = gl_TexCoord[0].st;
    
    st = (st - 0.5) * 2.0 + 0.5;
    float v0 = texture2DRect(tex, st).x;
    float v1 = texture2DRect(tex, st + vec2(1.0, 0.0)).x;
    float v2 = texture2DRect(tex, st + vec2(1.0, 1.0)).x;
    float v3 = texture2DRect(tex, st + vec2(0.0, 1.0)).x;
    
    float val = (v0 + v1 + v2 + v3) / 4.0;
//    float val = max(max(v0, v1), max(v2, v3));
    gl_FragColor = vec4(val);
}